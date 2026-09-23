#include <catch2/catch_test_macros.hpp>
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
#include "reflection/Reflection.h"
#include "codegen/CppGenerator.h"
#include <cstdio>
#include <fstream>

TEST_CASE("byte order inheritance is resolved canonically", "[byte-order]") {
    const char* path = "byte_order_closure.embx";
    std::ofstream g(path);
    g << R"(@ endian big
struct Outer {
  a: u16;
  b: u32 little;
  c: u16 native;
}
struct Inner little { x: u16; y: u32; }
struct Explicit little { x: u16; }
)";
    g.close();

    std::string err;
    auto ast = embx::parser::parseFile(path, err);
    REQUIRE((ast && err.empty()));
    REQUIRE(embx::semantic::analyze(*ast, err));
    auto ir = embx::ir::lower(*ast, err);
    REQUIRE((ir && err.empty()));
    auto plan = embx::plan::build(*ir, err);
    REQUIRE((plan && err.empty()));

    const auto outerId = plan->symbolTable.findId("Outer");
    REQUIRE(outerId != embx::core::InvalidSymbolId);
    const auto& outer = plan->structs.at(plan->structIndexBySymbol.at(outerId));
    REQUIRE(outer.endian == embx::plan::Endian::Big);

    auto* a = dynamic_cast<embx::plan::Field*>(outer.members.at(0).get());
    auto* b = dynamic_cast<embx::plan::Field*>(outer.members.at(1).get());
    auto* c = dynamic_cast<embx::plan::Field*>(outer.members.at(2).get());
    REQUIRE((a && b && c));
    REQUIRE(a->endian == embx::plan::Endian::Big);
    REQUIRE(b->endian == embx::plan::Endian::Little);
    REQUIRE(c->endian == embx::plan::Endian::Native);

    const auto explicitId = plan->symbolTable.findId("Explicit");
    REQUIRE(explicitId != embx::core::InvalidSymbolId);
    REQUIRE(plan->structs.at(plan->structIndexBySymbol.at(explicitId)).endian == embx::plan::Endian::Little);

    const auto reflected = embx::reflection::inspectStruct(*plan, "Outer");
    REQUIRE(reflected.has_value());
    REQUIRE(reflected->endian == embx::plan::Endian::Big);

    embx::codegen::Output out;
    REQUIRE(embx::codegen::generateCpp(*plan, out, err));
    REQUIRE(err.empty());
    REQUIRE(out.source.find("hostLittle()") != std::string::npos);
    REQUIRE(out.source.find("hostLittle() ? false : true") != std::string::npos);

    std::remove(path);
}

TEST_CASE("byte order precedence is module then struct then field", "[byte-order]") {
    const char* path = "byte_order_precedence.embx";
    std::ofstream g(path);
    g << R"(@ endian little
struct S big { a: u16; b: u16 little; c: u16 native; }
)";
    g.close();
    std::string err;
    auto ast = embx::parser::parseFile(path, err); REQUIRE((ast && err.empty()));
    REQUIRE(embx::semantic::analyze(*ast, err));
    auto ir = embx::ir::lower(*ast, err); REQUIRE((ir && err.empty()));
    auto p = embx::plan::build(*ir, err); REQUIRE((p && err.empty()));
    const auto id = p->symbolTable.findId("S");
    const auto& s = p->structs.at(p->structIndexBySymbol.at(id));
    REQUIRE(s.endian == embx::plan::Endian::Big);
    REQUIRE(dynamic_cast<embx::plan::Field*>(s.members.at(0).get())->endian == embx::plan::Endian::Big);
    REQUIRE(dynamic_cast<embx::plan::Field*>(s.members.at(1).get())->endian == embx::plan::Endian::Little);
    REQUIRE(dynamic_cast<embx::plan::Field*>(s.members.at(2).get())->endian == embx::plan::Endian::Native);
    std::remove(path);
}
