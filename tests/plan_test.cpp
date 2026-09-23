#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include "ir/IrBuilder.h"
#include "parser/ParserDriver.h"
#include "plan/PlanBuilder.h"
#include "plan/LayoutGraph.h"
#include "semantic/SemanticAnalyzer.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>

static void fail(const std::string& s){ FAIL(s); }
static std::unique_ptr<embx::plan::Field> field(const char* name, const char* type){ auto f=std::make_unique<embx::plan::Field>(); f->name=name; f->type.kind=embx::core::TypeKind::Primitive; f->type.name=type; return f; }
TEST_CASE("generated size properties", "[plan][size-properties]") {
    const char* path="generated_size_properties.embx";
    std::ofstream g(path);
    g << "struct Exact { a: u16; let n = $size_in_bytes; let lo = $min_size_in_bytes; let hi = $max_size_in_bytes; }";
    g.close();
    std::string err;
    auto a=embx::parser::parseFile(path,err); REQUIRE((a&&err.empty()));
    REQUIRE(embx::semantic::analyze(*a,err));
    auto ir=embx::ir::lower(*a,err); REQUIRE((ir&&err.empty()));
    auto p=embx::plan::build(*ir,err); REQUIRE((p&&err.empty()));
    const auto& s=p->structs[p->structIndexBySymbol.at(p->symbolTable.findId("Exact"))];
    REQUIRE(s.minSizeInBytes==2);
    REQUIRE((s.sizeInBytes && *s.sizeInBytes==2));
    REQUIRE((s.maxSizeInBytes && *s.maxSizeInBytes==2));
    auto* n=dynamic_cast<embx::plan::Virtual*>(s.members[1].get());
    auto* lo=dynamic_cast<embx::plan::Virtual*>(s.members[2].get());
    auto* hi=dynamic_cast<embx::plan::Virtual*>(s.members[3].get());
    REQUIRE((n&&lo&&hi));
    REQUIRE(n->expression->reference.id==embx::core::BuiltinSizeInBytesSymbolId);
    REQUIRE(lo->expression->reference.id==embx::core::BuiltinMinSizeInBytesSymbolId);
    REQUIRE(hi->expression->reference.id==embx::core::BuiltinMaxSizeInBytesSymbolId);
    std::remove(path);
}

TEST_CASE("plan", "[plan]") {
    const char* path="plan_test.embx"; std::ofstream g(path);
    g<<R"(@ endian big
const N = 4;
const M = N + 2;
type Payload = bytes[N];
enum Kind: u8 { A = 1, B = 2 }
struct Header little { a: u16; b: u32 big; }
struct Packet {
  h: Header;
  count: u8;
  data: bytes[N];
  bits { x: u8 (3); y: u8 (5); }
  align(4);
  block body[M] { z: u16; }
  at(64) { q: u8; }
  variant v by count { 1: u16; 2: { k: u8; } default: Payload; }
})";
    g.close(); std::string err; auto a=embx::parser::parseFile(path,err); if(!a||!err.empty()) fail("parse: "+err);
    if(!embx::semantic::analyze(*a,err)) fail("semantic: "+err);
    auto ir=embx::ir::lower(*a,err);if(!ir||!err.empty()) fail("ir: "+err);
    auto p=embx::plan::build(*ir,err);if(!p||!err.empty()) fail("plan: "+err);
    // The executable plan must not depend on AST/IR lifetime.
    ir.reset(); a.reset();
    REQUIRE(p->endian==embx::plan::Endian::Big);
    REQUIRE(p->constants[p->constantIndexBySymbol.at(p->symbolTable.findId("N"))].value==embx::runtime::Value(int64_t(4)));
    REQUIRE(p->constants[p->constantIndexBySymbol.at(p->symbolTable.findId("M"))].value==embx::runtime::Value(int64_t(6)));
    REQUIRE((p->aliases.size()==1 && p->aliases[0].target.name=="bytes"));
    REQUIRE(p->structs.size()==2);
    auto& h=p->structs[p->structIndexBySymbol.at(p->symbolTable.findId("Header"))];
    REQUIRE(h.endian==embx::plan::Endian::Little); REQUIRE((h.staticSize && *h.staticSize==6));
    auto* ha=dynamic_cast<embx::plan::Field*>(h.members[0].get()); auto* hb=dynamic_cast<embx::plan::Field*>(h.members[1].get());
    REQUIRE((ha&&hb&&ha->endian==embx::plan::Endian::Little&&hb->endian==embx::plan::Endian::Big&&ha->staticOffset==0&&hb->staticOffset==2));
    auto& s=p->structs[p->structIndexBySymbol.at(p->symbolTable.findId("Packet"))]; REQUIRE(!s.staticSize);
    auto* f0=dynamic_cast<embx::plan::Field*>(s.members[0].get()); auto* f1=dynamic_cast<embx::plan::Field*>(s.members[1].get()); auto* bits=dynamic_cast<embx::plan::Bits*>(s.members[3].get());
    REQUIRE((f0&&f0->staticOffset==0&&f0->staticSize==6)); REQUIRE((f1&&f1->staticOffset==6&&f1->endian==embx::plan::Endian::Big));
    REQUIRE((bits&&bits->totalBits==8&&bits->storageBytes==1));
    auto* block=dynamic_cast<embx::plan::Block*>(s.members[5].get()); REQUIRE((block&&block->staticSize==6));
    auto* at=dynamic_cast<embx::plan::At*>(s.members[6].get()); REQUIRE((at&&at->staticOffset==64));
    auto* var=dynamic_cast<embx::plan::Variant*>(s.members[7].get()); REQUIRE((var&&var->cases.size()==2&&var->hasDefault&&var->defaultType&&var->defaultType->name=="bytes")); REQUIRE(var->defaultType->dimensions.size()==1); REQUIRE(var->defaultType->dimensions[0].expression); REQUIRE(var->defaultType->dimensions[0].expression->text=="4");
    const char* fixed="plan_fixed_layout.embx"; std::ofstream xg(fixed);
    xg << "struct Fixed { a: u8; block body[4] { x: u8; } at(8) { y: u16; } z: u8; }"; xg.close();
    std::string xerr; auto xa=embx::parser::parseFile(fixed,xerr); REQUIRE((xa&&xerr.empty()));
    auto xir=embx::ir::lower(*xa,xerr); REQUIRE((xir&&xerr.empty())); auto xp=embx::plan::build(*xir,xerr);
    REQUIRE((xp&&xerr.empty())); auto& fx=xp->structs[xp->structIndexBySymbol.at(xp->symbolTable.findId("Fixed"))];
    REQUIRE((fx.staticSize && *fx.staticSize==10));
    REQUIRE(dynamic_cast<embx::plan::Block*>(fx.members[1].get())->staticSize==4);
    REQUIRE(dynamic_cast<embx::plan::At*>(fx.members[2].get())->staticOffset==8);
    auto* fy=dynamic_cast<embx::plan::Field*>(fx.members[3].get()); REQUIRE((fy && fy->staticOffset==5));
    std::remove(fixed);
    std::remove(path);
    const char* fwd="plan_forward.embx"; std::ofstream fg(fwd); fg << "computed A = B + 1; const B = 4; struct S { x: u8[A]; }"; fg.close();
    std::string ferr; auto fa=embx::parser::parseFile(fwd,ferr); REQUIRE((fa&&ferr.empty())); auto fir=embx::ir::lower(*fa,ferr); REQUIRE((fir&&ferr.empty())); auto fp=embx::plan::build(*fir,ferr); REQUIRE((fp&&ferr.empty())); REQUIRE(fp->constants[fp->constantIndexBySymbol.at(fp->symbolTable.findId("A"))].value==embx::runtime::Value(int64_t(5))); REQUIRE(fp->constants[0].computed); std::remove(fwd);
    // Canonical layout bounds: exact, bounded and unbounded are derived from
    // executable type shape and operations, not from a second size model.
    {
        embx::plan::Module bp;
        embx::plan::Type u32 = embx::plan::Type{};
        u32.kind = embx::core::TypeKind::Primitive; u32.name = "u32";
        auto exact = embx::plan::layoutBounds(bp, u32, ferr);
        REQUIRE((ferr.empty() && exact.exact() && exact.minSize == 4 && *exact.maxSize == 4));

        embx::plan::Type array; array.kind = embx::core::TypeKind::Primitive; array.name = "u32";
        array.dimensions.push_back(embx::core::Dimension::fixed(std::make_unique<embx::core::Expr>()));
        array.dimensions.back().expression->kind = embx::core::ExprKind::Literal;
        array.dimensions.back().expression->text = "4";
        ferr.clear(); auto exactArray = embx::plan::layoutBounds(bp, array, ferr);
        REQUIRE((ferr.empty() && exactArray.exact() && exactArray.minSize == 16 && *exactArray.maxSize == 16));

        embx::plan::Type dynamic; dynamic.kind = embx::core::TypeKind::Primitive; dynamic.name = "u32";
        auto dynExpr = std::make_unique<embx::core::Expr>();
        dynExpr->kind = embx::core::ExprKind::Identifier; dynExpr->text = "count";
        dynamic.dimensions.push_back(embx::core::Dimension::dynamic(std::move(dynExpr)));
        ferr.clear(); auto unbounded = embx::plan::layoutBounds(bp, dynamic, ferr);
        REQUIRE((ferr.empty() && unbounded.minSize == 0 && !unbounded.maxSize && unbounded.classification() == embx::plan::LayoutClass::Unbounded));

        embx::plan::Struct bs; bs.name = "Bounds";
        auto one = std::make_unique<embx::plan::Field>(); one->name = "a"; one->type.kind = embx::core::TypeKind::Primitive; one->type.name = "u8";
        auto two = std::make_unique<embx::plan::Field>(); two->name = "b"; two->type.kind = embx::core::TypeKind::Primitive; two->type.name = "u16";
        bs.members.push_back(std::move(one)); bs.members.push_back(std::move(two));
        auto sid = embx::test::addStruct(bp, std::move(bs));
        embx::plan::Type named; named.kind = embx::core::TypeKind::Named; named.name = "Bounds"; named.reference.id = sid;
        ferr.clear(); auto structBounds = embx::plan::layoutBounds(bp, named, ferr);
        REQUIRE((ferr.empty() && structBounds.exact() && structBounds.minSize == 3 && *structBounds.maxSize == 3));

        embx::plan::Struct vs; vs.name = "Choice";
        auto variant = std::make_unique<embx::plan::Variant>(); variant->name = "v";
        embx::plan::VariantCase c1; c1.tag = std::uint64_t(1); c1.type.emplace(); c1.type->kind = embx::core::TypeKind::Primitive; c1.type->name = "u32";
        embx::plan::VariantCase c2; c2.tag = std::uint64_t(2); c2.type = embx::plan::Type{}; c2.type->kind = embx::core::TypeKind::Primitive; c2.type->name = "u8";
        variant->cases.push_back(std::move(c1)); variant->cases.push_back(std::move(c2));
        vs.members.push_back(std::move(variant));
        auto vsid = embx::test::addStruct(bp, std::move(vs));
        embx::plan::Type vnamed; vnamed.kind = embx::core::TypeKind::Named; vnamed.name = "Choice"; vnamed.reference.id = vsid;
        ferr.clear(); auto variantBounds = embx::plan::layoutBounds(bp, vnamed, ferr);
        REQUIRE((ferr.empty() && variantBounds.classification() == embx::plan::LayoutClass::Bounded && variantBounds.minSize == 1 && *variantBounds.maxSize == 4));

        // Absolute at() is composed by extent, not by sequential addition.
        embx::plan::Module ap;
        embx::plan::Struct abs; abs.name = "Absolute";
        abs.members.push_back(field("head", "u8"));
        auto outerAt = std::make_unique<embx::plan::At>(); outerAt->staticOffset = 64;
        outerAt->members.push_back(field("word", "u32"));
        auto nestedAt = std::make_unique<embx::plan::At>(); nestedAt->staticOffset = 100;
        nestedAt->members.push_back(field("tail", "u8"));
        outerAt->members.push_back(std::move(nestedAt));
        outerAt->members.push_back(field("after", "u8"));
        abs.members.push_back(std::move(outerAt));
        auto absId = embx::test::addStruct(ap, std::move(abs));
        embx::plan::Type absType; absType.kind = embx::core::TypeKind::Named; absType.name = "Absolute"; absType.reference.id = absId;
        ferr.clear(); auto absBounds = embx::plan::layoutBounds(ap, absType, ferr);
        REQUIRE((ferr.empty() && absBounds.exact() && absBounds.minSize == 101 && *absBounds.maxSize == 101));

        // Alignment is evaluated against the actual cursor inside an at() region.
        embx::plan::Module al;
        embx::plan::Struct aligned; aligned.name = "Aligned";
        auto alignedAt = std::make_unique<embx::plan::At>(); alignedAt->staticOffset = 5;
        alignedAt->members.push_back(field("a", "u8"));
        auto align = std::make_unique<embx::plan::Align>(); align->staticAlignment = 4;
        alignedAt->members.push_back(std::move(align));
        alignedAt->members.push_back(field("b", "u16"));
        aligned.members.push_back(std::move(alignedAt));
        auto alignedId = embx::test::addStruct(al, std::move(aligned));
        embx::plan::Type alignedType; alignedType.kind = embx::core::TypeKind::Named; alignedType.name = "Aligned"; alignedType.reference.id = alignedId;
        ferr.clear(); auto alignedBounds = embx::plan::layoutBounds(al, alignedType, ferr);
        REQUIRE((ferr.empty() && alignedBounds.exact() && alignedBounds.minSize == 10 && *alignedBounds.maxSize == 10));

        // A dynamic field inside at() preserves the known absolute lower bound
        // while correctly making the upper bound unbounded.
        embx::plan::Module da;
        embx::plan::Struct dynamicAt; dynamicAt.name = "DynamicAt";
        auto daAt = std::make_unique<embx::plan::At>(); daAt->staticOffset = 64;
        auto daField = field("data", "u32");
        auto daExpr = std::make_unique<embx::core::Expr>(); daExpr->kind = embx::core::ExprKind::Identifier; daExpr->text = "count";
        daField->type.dimensions.push_back(embx::core::Dimension::dynamic(std::move(daExpr)));
        daAt->members.push_back(std::move(daField));
        dynamicAt.members.push_back(std::move(daAt));
        auto daId = embx::test::addStruct(da, std::move(dynamicAt));
        embx::plan::Type daType; daType.kind = embx::core::TypeKind::Named; daType.name = "DynamicAt"; daType.reference.id = daId;
        ferr.clear(); auto daBounds = embx::plan::layoutBounds(da, daType, ferr);
        REQUIRE((ferr.empty() && daBounds.classification() == embx::plan::LayoutClass::Unbounded && daBounds.minSize == 64 && !daBounds.maxSize));

        // Overflow at an absolute extent is rejected rather than wrapping.
        embx::plan::Module ov;
        embx::plan::Struct overflow; overflow.name = "Overflow";
        auto ovAt = std::make_unique<embx::plan::At>(); ovAt->staticOffset = std::numeric_limits<embx::runtime::LayoutSize>::max();
        ovAt->members.push_back(field("x", "u8")); overflow.members.push_back(std::move(ovAt));
        auto ovId = embx::test::addStruct(ov, std::move(overflow));
        embx::plan::Type ovType; ovType.kind = embx::core::TypeKind::Named; ovType.name = "Overflow"; ovType.reference.id = ovId;
        ferr.clear(); (void)embx::plan::layoutBounds(ov, ovType, ferr);
        REQUIRE((ferr == "layout maximum overflow" || ferr == "layout minimum overflow"));
    }

    std::cout<<"Plan tests passed\n";
}



TEST_CASE("$next resolves to the end of the previous sequential operation", "[next]") {
    const char* path = "next_test.embx";
    std::ofstream g(path);
    g << R"(struct Packet {
  head: u8;
  at($next + 2) { value: u16; }
  tail: u8;
})";
    g.close();
    std::string err;
    auto a = embx::parser::parseFile(path, err);
    REQUIRE((a && err.empty()));
    REQUIRE(embx::semantic::analyze(*a, err));
    auto ir = embx::ir::lower(*a, err);
    REQUIRE((ir && err.empty()));
    auto p = embx::plan::build(*ir, err);
    REQUIRE((p && err.empty()));
    auto& s = p->structs[p->structIndexBySymbol.at(p->symbolTable.findId("Packet"))];
    auto* at = dynamic_cast<embx::plan::At*>(s.members[1].get());
    REQUIRE(at);
    REQUIRE((at->offset && at->offset->text == "$next+2"));
    REQUIRE((at->offset->left &&
             at->offset->left->reference.id == embx::core::BuiltinNextSymbolId));
    REQUIRE((at->staticOffset && *at->staticOffset == 3));
    auto* tail = dynamic_cast<embx::plan::Field*>(s.members[2].get());
    REQUIRE((tail && tail->staticOffset == 1));
    std::remove(path);
}

TEST_CASE("conditional fields semantic and plan closure", "[conditional]") {
    const char* path = "conditional_test.embx";
    std::ofstream g(path);
    g << R"(struct Packet {
  flags: u8;
  if (flags == 1) { value: u32; }
  else { small: u16; }
})";
    g.close();
    std::string err;
    auto a = embx::parser::parseFile(path, err);
    REQUIRE((a && err.empty()));
    REQUIRE(embx::semantic::analyze(*a, err));
    auto ir = embx::ir::lower(*a, err);
    REQUIRE((ir && err.empty()));
    REQUIRE(ir->structs.size() == 1);
    REQUIRE(ir->structs[0]->members.size() == 2);
    REQUIRE(ir->structs[0]->members[1]->kind == embx::ir::MemberKind::Conditional);
    auto irc = dynamic_cast<embx::ir::Conditional*>(ir->structs[0]->members[1].get());
    REQUIRE(irc);
    REQUIRE((irc->condition && irc->condition->type == embx::core::ExprType::Boolean));
    REQUIRE(irc->thenMembers.size() == 1);
    REQUIRE(irc->elseMembers.size() == 1);

    auto p = embx::plan::build(*ir, err);
    REQUIRE((p && err.empty()));
    auto& s = p->structs[p->structIndexBySymbol.at(p->symbolTable.findId("Packet"))];
    REQUIRE(s.members.size() == 2);
    auto* c = dynamic_cast<embx::plan::Conditional*>(s.members[1].get());
    REQUIRE(c);
    REQUIRE((c->condition && c->condition->type == embx::core::ExprType::Boolean));
    REQUIRE(c->thenMembers.size() == 1);
    REQUIRE(c->elseMembers.size() == 1);
    REQUIRE(c->thenMembers[0]->kind == embx::plan::OpKind::Field);
    REQUIRE(c->elseMembers[0]->kind == embx::plan::OpKind::Field);

    embx::plan::LayoutGraph graph;
    REQUIRE(embx::plan::buildLayoutGraph(*p, graph, err));
    const auto flagsId = irc->condition->left->reference.id;
    REQUIRE(flagsId != embx::core::InvalidSymbolId);
    REQUIRE(std::any_of(graph.edges.begin(), graph.edges.end(), [&](const auto& e) {
        return e.to == flagsId;
    }));

    std::string boundsErr;
    embx::plan::Type packet; packet.kind = embx::core::TypeKind::Named; packet.name = "Packet";
    packet.reference.id = s.symbol;
    auto b = embx::plan::layoutBounds(*p, packet, boundsErr);
    REQUIRE((boundsErr.empty() && b.classification() == embx::plan::LayoutClass::Bounded));
    REQUIRE(b.minSize == 3);
    REQUIRE((b.maxSize && *b.maxSize == 5));

    std::remove(path);
}

TEST_CASE("conditional fields reject unavailable branch symbols", "[conditional]") {
    const char* path = "conditional_scope_test.embx";
    std::ofstream g(path);
    g << R"(struct Bad {
  flag: u8;
  if (flag == 1) { hidden: u32; }
  after: u32[hidden];
})";
    g.close();
    std::string err;
    auto a = embx::parser::parseFile(path, err);
    REQUIRE((a && err.empty()));
    REQUIRE(embx::semantic::analyze(*a, err));
    auto ir = embx::ir::lower(*a, err);
    REQUIRE((ir && err.empty()));
    auto p = embx::plan::build(*ir, err);
    REQUIRE(!p);
    REQUIRE((err.find("unavailable") != std::string::npos || err.find("unknown identifier") != std::string::npos));
    std::remove(path);
}
