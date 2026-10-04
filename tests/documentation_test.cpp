#include <catch2/catch_test_macros.hpp>
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
#include "reflection/Reflection.h"
#include <fstream>
#include <cstdio>

TEST_CASE("documentation", "[documentation]") {
    const char* text =
        "/**\n"
        " * Module documentation.\n"
        " */\n"
        "attribute packed;\n"
        "/// Structure documentation.\n"
        "[packed] struct Packet {\n"
        "  /// Field documentation.\n"
        "  value: u8;\n"
        "  /** Block documentation. */\n"
        "  block body[2] {\n"
        "    /// Nested field.\n"
        "    x: u8;\n"
        "  }\n"
        "}\n"
        "\n"
        "/// Enum documentation.\n"
        "enum Mode {\n"
        "  /// Zero mode.\n"
        "  ZERO = 0,\n"
        "  /** One mode. */ ONE = 1\n"
        "}\n"
        "\n"
        "/// Alias documentation.\n"
        "type Byte = u8;\n"
        "\n"
        "/// Callback documentation.\n"
        "callback on_decode_documentation();\n";

    std::ofstream f("documentation_test.embx"); f << text; f.close();
    std::string err;
    auto m = embx::parser::parseFile("documentation_test.embx", err);
    REQUIRE((m && err.empty()));
    REQUIRE(m->documentation == "Module documentation.");
    REQUIRE(m->structs.size() == 1);
    REQUIRE(m->structs[0]->documentation == "Structure documentation.");
    REQUIRE(m->structs[0]->members.size() == 2);
    REQUIRE(m->structs[0]->members[0]->documentation == "Field documentation.");
    REQUIRE(m->structs[0]->members[1]->documentation == "Block documentation.");

    CAPTURE(err);
    REQUIRE(embx::semantic::analyze(*m, err));
    auto ir = embx::ir::lower(*m, err); REQUIRE((ir && err.empty()));
    auto p = embx::plan::build(*ir, err); REQUIRE((p && err.empty()));
    REQUIRE(p->structs[0].documentation == "Structure documentation.");
    REQUIRE(p->structs[0].members[0]->documentation == "Field documentation.");
    REQUIRE(p->structs[0].members[1]->documentation == "Block documentation.");
    auto r = embx::reflection::inspect(*p);
    REQUIRE(r.documentation == "Module documentation.");
    auto rs = embx::reflection::inspectStruct(*p, p->structs[0].name);
    REQUIRE(rs);
    REQUIRE(rs->documentation == "Structure documentation.");
    REQUIRE(rs->members[0].documentation == "Field documentation.");
    REQUIRE(rs->members[1].documentation == "Block documentation.");
    REQUIRE(rs->members[1].nested[0].documentation == "Nested field.");
    REQUIRE(r.enumItems.size() == 1);
    REQUIRE(r.enumItems[0].size() == 2);
    REQUIRE(r.enumItems[0][0].name == "ZERO");
    REQUIRE(r.enumItems[0][0].documentation == "Zero mode.");
    REQUIRE(r.enumItems[0][1].name == "ONE");
    REQUIRE(r.enumItems[0][1].documentation == "One mode.");

    std::remove("documentation_test.embx");

    const char* lineDocText =
        "/// Module line documentation.\n"
        "/// Structure documentation.\n"
        "struct LineDoc { value: u8; }\n";
    std::ofstream lf("documentation_module_lines.embx"); lf << lineDocText; lf.close();
    auto lm = embx::parser::parseFile("documentation_module_lines.embx", err);
    REQUIRE((lm && err.empty()));
    REQUIRE(lm->documentation == "Module line documentation.");
    REQUIRE(lm->structs.size() == 1);
    REQUIRE(lm->structs[0]->documentation == "Structure documentation.");
    std::remove("documentation_module_lines.embx");
}
