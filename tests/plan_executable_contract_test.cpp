#include <catch2/catch_test_macros.hpp>
#include "ir/IrBuilder.h"
#include "parser/ParserDriver.h"
#include "plan/PlanBuilder.h"
#include "semantic/SemanticAnalyzer.h"
#include <fstream>
#include <cstdio>

TEST_CASE("plan is finalized as an executable contract") {
    const char* path = "plan_executable_contract.embx";
    std::ofstream f(path);
    f << R"(const N = 4;
struct Packet {
  count: u8;
  data: bytes[N];
  block body[count] { value: u16; }
  align(4);
  at(16) { tail: u8; }
  variant body_by_count by count { 0: u8; default: { x: u16; } }
})";
    f.close();

    std::string err;
    auto ast = embx::parser::parseFile(path, err);
    REQUIRE(ast);
    REQUIRE(err.empty());
    auto ir = embx::ir::lower(*ast, err);
    REQUIRE(ir);
    REQUIRE(err.empty());
    auto plan = embx::plan::build(*ir, err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    // The plan must retain canonical identities and finalized metadata after
    // the source AST/IR are destroyed. Runtime consumers do not need either.
    ir.reset();
    ast.reset();

    const auto sid = plan->symbolTable.findId("Packet");
    REQUIRE(sid != embx::core::InvalidSymbolId);
    REQUIRE(plan->structIndexBySymbol.at(sid) == 0);
    REQUIRE(plan->structs[0].symbol == sid);

    const auto* data = dynamic_cast<const embx::plan::Field*>(plan->structs[0].members[1].get());
    REQUIRE(data);
    REQUIRE(data->staticLength.has_value());
    REQUIRE(*data->staticLength == 4);

    // Static dimensions are finalized in Plan; runtime must not evaluate the
    // constant expression again.
    REQUIRE(data->type.name == "bytes");
    REQUIRE(data->type.dimensions.size() == 1);
    REQUIRE(data->type.dimensions[0].kind == embx::core::Dimension::Kind::Fixed);
    REQUIRE(data->type.dimensions[0].expression);
    REQUIRE(data->type.dimensions[0].expression->kind == embx::core::ExprKind::Literal);
    REQUIRE(data->type.dimensions[0].expression->text == "4");

    const auto* block = dynamic_cast<const embx::plan::Block*>(plan->structs[0].members[2].get());
    REQUIRE(block);
    REQUIRE(block->size);

    const auto* align = dynamic_cast<const embx::plan::Align*>(plan->structs[0].members[3].get());
    REQUIRE(align);
    REQUIRE(align->alignment);
    REQUIRE(align->staticAlignment.has_value());
    REQUIRE(*align->staticAlignment == 4);

    const auto* at = dynamic_cast<const embx::plan::At*>(plan->structs[0].members[4].get());
    REQUIRE(at);
    REQUIRE(at->offset);
    REQUIRE(at->staticOffset.has_value());
    REQUIRE(*at->staticOffset == 16);

    const auto* variant = dynamic_cast<const embx::plan::Variant*>(plan->structs[0].members[5].get());
    REQUIRE(variant);
    REQUIRE(variant->discriminator);
    REQUIRE(variant->cases.size() == 1);
    REQUIRE(variant->hasDefault);

    std::remove(path);
}

TEST_CASE("plan retains valid executable bit metadata") {
    embx::ir::Module ir;
    const auto sid = ir.symbolTable.declare(embx::core::SymbolKind::Struct, "Packet");
    auto st = std::make_unique<embx::ir::Struct>();
    st->symbol = sid;
    st->name = "Packet";
    auto bits = std::make_unique<embx::ir::Bits>();
    auto bf = std::make_unique<embx::ir::Field>();
    const auto fid = ir.symbolTable.declareScoped(embx::core::SymbolKind::Field, "x");
    bf->symbol = fid;
    bf->name = "x";
    bf->type.kind = embx::core::TypeKind::Primitive;
    bf->type.name = "u8";
    bf->bits = 8;
    bits->fields.push_back(std::move(bf));
    st->members.push_back(std::move(bits));
    ir.structs.push_back(std::move(st));
    ir.order.push_back({embx::ir::TopLevelRef::Kind::Struct, 0});

    std::string err;
    auto plan = embx::plan::build(ir, err);
    REQUIRE(plan);
    REQUIRE(err.empty());
}

TEST_CASE("malformed IR SymbolId/name identity is rejected before Plan execution") {
    embx::ir::Module ir;
    const auto sid = ir.symbolTable.declare(embx::core::SymbolKind::Struct, "Packet");
    auto st = std::make_unique<embx::ir::Struct>();
    st->symbol = sid;
    st->name = "Packet";

    const auto fid = ir.symbolTable.declareScoped(embx::core::SymbolKind::Field, "actual");
    auto field = std::make_unique<embx::ir::Field>();
    field->symbol = fid;
    field->name = "tampered";
    field->type.kind = embx::core::TypeKind::Primitive;
    field->type.name = "u8";
    st->members.push_back(std::move(field));
    ir.structs.push_back(std::move(st));
    ir.order.push_back({embx::ir::TopLevelRef::Kind::Struct, 0});

    std::string err;
    auto plan = embx::plan::build(ir, err);
    REQUIRE_FALSE(plan);
    REQUIRE(err.find("SymbolId/name mismatch") != std::string::npos);
}
