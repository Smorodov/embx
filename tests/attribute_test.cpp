#include <catch2/catch_test_macros.hpp>
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
#include "reflection/Reflection.h"
#include <iostream>
#include <fstream>
#include <cstdio>

static bool accepts(const char* text, const char* needle = nullptr) {
    std::string err;
    { std::ofstream f("attribute_test.embx"); f << text; }
    auto m = embx::parser::parseFile("attribute_test.embx", err);
    if (!m || !err.empty()) return false;
    if (!embx::semantic::analyze(*m, err)) return false;
    auto ir = embx::ir::lower(*m, err);
    if (!ir || !err.empty()) return false;
    auto p = embx::plan::build(*ir, err);
    if (!p || !err.empty()) return false;
    if (needle) {
        auto r = embx::reflection::inspect(*p);
        return !r.structAttributes.empty() && !r.structAttributes[0].empty() && r.structAttributes[0][0] == needle;
    }
    return true;
}

static bool rejects(const char* text, const char* needle) {
    std::string err;
    { std::ofstream f("attribute_negative.embx"); f << text; }
    auto m = embx::parser::parseFile("attribute_negative.embx", err);
    if (!m || !err.empty()) return false;
    if (embx::semantic::analyze(*m, err)) return false;
    return err.find(needle) != std::string::npos;
}

TEST_CASE("attribute", "[attribute]") {
    REQUIRE(accepts("attribute packed; [packed] struct S { x: u8; }", "packed"));
    REQUIRE(accepts("attribute title: string; [title=\"payload\"] struct S { x: u8; }"));
    REQUIRE(accepts("attribute width: u8; type T = u8; [width=8] struct S { [width=1] x: T; }"));
    {
        std::string err;
        std::ofstream f("attribute_reflection.embx");
        f << "attribute packed; attribute width: u8; struct S { [width=8] x: u8; }";
        f.close();
        auto m = embx::parser::parseFile("attribute_reflection.embx", err);
        REQUIRE(m);
        REQUIRE(embx::semantic::analyze(*m, err));
        auto ir = embx::ir::lower(*m, err);
        REQUIRE(ir);
        auto p = embx::plan::build(*ir, err);
        REQUIRE(p);
        auto r = embx::reflection::inspect(*p);
        REQUIRE(r.attributeDefinitions.size() == 2);
        REQUIRE(r.attributeDefinitions[0].name == "packed");
        REQUIRE(r.attributeDefinitions[0].type == embx::core::AttributeType::Marker);
        REQUIRE(r.attributeDefinitions[1].name == "width");
        REQUIRE(r.attributeDefinitions[1].type == embx::core::AttributeType::Integer);
        std::remove("attribute_reflection.embx");
    }
    REQUIRE(rejects("attribute packed; [packed=true] struct S { x: u8; }", "marker attribute cannot have a value"));
    REQUIRE(rejects("attribute width: u8; [width=1.5] struct S { x: u8; }", "attribute requires integer value"));
    REQUIRE(rejects("attribute title: string; [title=1] struct S { x: u8; }", "attribute requires string value"));
    REQUIRE(rejects("attribute packed; [packed, packed] struct S { x: u8; }", "duplicate attribute"));
    REQUIRE(rejects("[missing] struct S { x: u8; }", "unknown attribute"));
    REQUIRE(rejects("attribute width: u8; [width=flag] struct S { x: u8; }", "attribute requires integer value"));
    REQUIRE(rejects("attribute gain: f32; [gain=1] struct S { x: u8; }", "attribute requires floating-point value"));
    std::remove("attribute_test.embx");
    std::remove("attribute_negative.embx");
}
