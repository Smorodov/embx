#include <catch2/catch_test_macros.hpp>
#include "core/Symbol.h"
#include "core/Resolver.h"
#include "ast/Ast.h"
#include "ir/IrBuilder.h"
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include <string>
#include <fstream>
#include <cstdio>

TEST_CASE("symbol ids are deterministic and opaque") {
    embx::core::SymbolTable t;
    const auto a=t.declare(embx::core::SymbolKind::Struct,"A");
    const auto b=t.declare(embx::core::SymbolKind::Constant,"B");
    REQUIRE(a == 1);
    REQUIRE(b == 2);
    REQUIRE(t.findId("A") == a);
    REQUIRE(t.find(a)->name == "A");
    REQUIRE(t.declare(embx::core::SymbolKind::Enum,"A") == embx::core::InvalidSymbolId);
}

TEST_CASE("scope resolves locally then through parent") {
    embx::core::SymbolTable t;
    auto a=t.declare(embx::core::SymbolKind::Struct,"A");
    auto b=t.declare(embx::core::SymbolKind::Constant,"B");
    embx::core::Scope root;
    embx::core::Scope child(&root);
    REQUIRE(root.declare("A",a));
    REQUIRE(child.declare("B",b));
    REQUIRE(child.resolve("A") == a);
    REQUIRE(child.resolve("B") == b);
    REQUIRE(child.resolve("missing") == embx::core::InvalidSymbolId);
}

TEST_CASE("IR declarations receive deterministic symbol identities") {
    embx::ast::Module a;
    embx::ast::Struct st; st.name = "A";
    embx::ast::Const c; c.name = "B";
    c.expr = std::make_unique<embx::ast::Expr>();
    c.expr->kind = embx::core::ExprKind::Literal;
    c.expr->text = "1";
    a.structs.push_back(std::make_unique<embx::ast::Struct>(std::move(st)));
    a.constants.push_back(std::move(c));
    a.order.push_back({embx::ast::TopLevelRef::Kind::Struct, 0});
    a.order.push_back({embx::ast::TopLevelRef::Kind::Const, 0});

    std::string err;
    auto ir=embx::ir::lower(a,err); REQUIRE(ir); REQUIRE(err.empty());
    REQUIRE(ir->structs.size()==1);
    REQUIRE(ir->constants.size()==1);
    REQUIRE(ir->structs[0]->symbol==1);
    REQUIRE(ir->constants[0].symbol==2);
    REQUIRE(ir->symbolTable.findId("A")==1);
    REQUIRE(ir->symbolTable.findId("B")==2);
}

TEST_CASE("canonical expressions resolve visible symbols") {
    embx::ast::Module a;

    embx::ast::Const c;
    c.name = "N";
    c.expr = std::make_unique<embx::ast::Expr>();
    c.expr->kind = embx::core::ExprKind::Literal;
    c.expr->text = "3";
    a.constants.push_back(std::move(c));
    a.order.push_back({embx::ast::TopLevelRef::Kind::Const, 0});

    auto s = std::make_unique<embx::ast::Struct>();
    s->name = "S";
    embx::ast::Field f;
    f.name = "n";
    f.type.name = "u8";
    s->members.push_back(std::make_unique<embx::ast::Field>(std::move(f)));
    embx::ast::Field payload;
    payload.name = "payload";
    payload.type.name = "u8";
    payload.modifiers.push_back({});
    payload.modifiers.back().kind = embx::ast::ModifierKind::Length;
    payload.modifiers.back().expr = std::make_unique<embx::ast::Expr>();
    payload.modifiers.back().expr->kind = embx::core::ExprKind::Identifier;
    payload.modifiers.back().expr->text = "n";
    s->members.push_back(std::make_unique<embx::ast::Field>(std::move(payload)));
    a.structs.push_back(std::move(s));
    a.order.push_back({embx::ast::TopLevelRef::Kind::Struct, 0});

    std::string err;
    auto ir = embx::ir::lower(a, err);
    REQUIRE(ir);
    REQUIRE(err.empty());
    REQUIRE(ir->structs[0]->members.size() == 2);

    auto* first = dynamic_cast<embx::ir::Field*>(ir->structs[0]->members[0].get());
    auto* second = dynamic_cast<embx::ir::Field*>(ir->structs[0]->members[1].get());
    REQUIRE(first);
    REQUIRE(second);
    REQUIRE(first->symbol != embx::core::InvalidSymbolId);
    REQUIRE(second->symbol != embx::core::InvalidSymbolId);
    REQUIRE(second->type.dimensions.size() == 1);
    REQUIRE(second->type.dimensions[0].expression);
    REQUIRE(second->type.dimensions[0].expression->reference.id == first->symbol);
}

TEST_CASE("forward field reference retains canonical SymbolId for later semantic diagnostics") {
    embx::ast::Module a;
    auto s = std::make_unique<embx::ast::Struct>();
    s->name = "S";

    embx::ast::Field first;
    first.name = "payload";
    first.type.name = "u8";
    first.modifiers.push_back({});
    first.modifiers.back().kind = embx::ast::ModifierKind::Length;
    first.modifiers.back().expr = std::make_unique<embx::ast::Expr>();
    first.modifiers.back().expr->kind = embx::core::ExprKind::Identifier;
    first.modifiers.back().expr->text = "later";
    s->members.push_back(std::make_unique<embx::ast::Field>(std::move(first)));

    embx::ast::Field later;
    later.name = "later";
    later.type.name = "u8";
    s->members.push_back(std::make_unique<embx::ast::Field>(std::move(later)));

    a.structs.push_back(std::move(s));
    a.order.push_back({embx::ast::TopLevelRef::Kind::Struct, 0});

    std::string err;
    auto ir = embx::ir::lower(a, err);
    REQUIRE(ir);
    REQUIRE(err.empty());
    auto* field = dynamic_cast<embx::ir::Field*>(ir->structs[0]->members[0].get());
    REQUIRE(field);
    REQUIRE(field->type.dimensions.size() == 1);
    REQUIRE(field->type.dimensions[0].expression);
    const auto* laterField = dynamic_cast<const embx::ir::Field*>(ir->structs[0]->members[1].get());
    REQUIRE(laterField);
    REQUIRE(laterField->symbol != embx::core::InvalidSymbolId);
    REQUIRE(field->type.dimensions[0].expression->reference.id == laterField->symbol);
}


TEST_CASE("scope-owned field identities allow sibling reuse") {
    embx::ast::Module a;

    auto sa = std::make_unique<embx::ast::Struct>();
    sa->name = "A";
    embx::ast::Field fa;
    fa.name = "value";
    fa.type.name = "u8";
    sa->members.push_back(std::make_unique<embx::ast::Field>(std::move(fa)));
    a.structs.push_back(std::move(sa));
    a.order.push_back({embx::ast::TopLevelRef::Kind::Struct, 0});

    auto sb = std::make_unique<embx::ast::Struct>();
    sb->name = "B";
    embx::ast::Field fb;
    fb.name = "value";
    fb.type.name = "u16";
    sb->members.push_back(std::make_unique<embx::ast::Field>(std::move(fb)));
    a.structs.push_back(std::move(sb));
    a.order.push_back({embx::ast::TopLevelRef::Kind::Struct, 1});

    std::string err;
    auto ir = embx::ir::lower(a, err);
    REQUIRE((ir && err.empty()));
    REQUIRE(ir->structs.size() == 2);
    auto* first = dynamic_cast<embx::ir::Field*>(ir->structs[0]->members[0].get());
    auto* second = dynamic_cast<embx::ir::Field*>(ir->structs[1]->members[0].get());
    REQUIRE(first);
    REQUIRE(second);
    REQUIRE(first->name == "value");
    REQUIRE(second->name == "value");
    REQUIRE(first->symbol != embx::core::InvalidSymbolId);
    REQUIRE(second->symbol != embx::core::InvalidSymbolId);
    REQUIRE(first->symbol != second->symbol);
}


TEST_CASE("enum items have canonical SymbolIds") {
    const char* path = "symbol_identity_enum_item.embx";
    std::ofstream f(path);
    f << "enum E: u8 { A = 1 } struct S { x: u8[A]; }";
    f.close();
    std::string err;
    auto ast = embx::parser::parseFile(path, err);
    REQUIRE(ast);
    REQUIRE(err.empty());
    REQUIRE(embx::semantic::analyze(*ast, err));
    auto ir = embx::ir::lower(*ast, err);
    REQUIRE(ir);
    REQUIRE(err.empty());
    const auto id = ir->symbolTable.findId("E::A");
    REQUIRE(id != embx::core::InvalidSymbolId);
    const auto* sym = ir->symbolTable.find(id);
    REQUIRE(sym != nullptr);
    REQUIRE(sym->kind == embx::core::SymbolKind::EnumItem);
    REQUIRE(ir->enums[0].items[0].symbol == id);
    REQUIRE(ir->structs[0]->members[0]->kind == embx::ir::MemberKind::Field);
    const auto* field = static_cast<const embx::ir::Field*>(ir->structs[0]->members[0].get());
    REQUIRE(field->type.dimensions.size() == 1);
    REQUIRE(field->type.dimensions[0].expression->reference.id == id);
    std::remove(path);
}


TEST_CASE("canonical NameResolver handles qualified names and shadowing") {
    embx::core::SymbolTable table;
    const auto moduleType = table.declare(embx::core::SymbolKind::Struct, "app::Packet");
    const auto moduleConst = table.declare(embx::core::SymbolKind::Constant, "app::SIZE");
    const auto field = table.declareScoped(embx::core::SymbolKind::Field, "SIZE");

    embx::core::Scope module;
    REQUIRE(module.declare("app::Packet", moduleType));
    REQUIRE(module.declare("app::SIZE", moduleConst));
    embx::core::Scope nested(&module);
    REQUIRE(nested.declare("SIZE", field));

    embx::core::NameResolver resolver(table, &nested);
    REQUIRE(resolver.resolve("SIZE") == field);
    REQUIRE(resolver.resolve("app::SIZE") == moduleConst);
    REQUIRE(resolver.resolve("app::Packet") == moduleType);
    REQUIRE(resolver.resolve("missing") == embx::core::InvalidSymbolId);

    embx::core::SymbolId out = embx::core::InvalidSymbolId;
    std::string err;
    REQUIRE(resolver.resolveRequired("app::Packet", out, err));
    REQUIRE(out == moduleType);
    REQUIRE(err.empty());
    REQUIRE_FALSE(resolver.resolveRequired("missing", out, err));
    REQUIRE(err == "unresolved name: missing");
}

TEST_CASE("IR lowering does not resolve a field through its own declaration") {
    embx::ast::Module a;
    auto s = std::make_unique<embx::ast::Struct>();
    s->name = "S";
    embx::ast::Field f;
    f.name = "x";
    f.type.name = "x";
    s->members.push_back(std::make_unique<embx::ast::Field>(std::move(f)));
    a.structs.push_back(std::move(s));
    a.order.push_back({embx::ast::TopLevelRef::Kind::Struct, 0});

    std::string err;
    auto ir = embx::ir::lower(a, err);
    REQUIRE(ir);
    REQUIRE(err.empty());
    const auto* field = dynamic_cast<const embx::ir::Field*>(ir->structs[0]->members[0].get());
    REQUIRE(field);
    REQUIRE(field->type.kind == embx::core::TypeKind::Named);
    REQUIRE(field->type.reference.id == embx::core::InvalidSymbolId);
}
