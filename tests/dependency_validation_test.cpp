#include <catch2/catch_test_macros.hpp>
#include "ir/IrBuilder.h"
#include "parser/ParserDriver.h"
#include "plan/PlanBuilder.h"
#include "semantic/SemanticAnalyzer.h"
#include <cstdio>
#include <fstream>
#include <iostream>

static bool fails(const char* text, const char* needle) {
    const char* path = "dependency_validation.embx";
    std::ofstream(path) << text;
    std::string err;
    auto ast = embx::parser::parseFile(path, err);
    if (!ast) { std::remove(path); return false; }
    if (!embx::semantic::analyze(*ast, err)) { std::remove(path); return false; }
    auto ir = embx::ir::lower(*ast, err);
    if (!ir) { std::remove(path); return false; }
    auto p = embx::plan::build(*ir, err);
    std::remove(path);
    return !p && err.find(needle) != std::string::npos;
}

TEST_CASE("dependency validation", "[dependency_validation]") {
    REQUIRE(fails("struct S { n: u8[count]; count: u8; }", "declared later"));
    REQUIRE(fails("struct S { n: u8; x: u8[n + missing]; }", "unknown identifier"));
    REQUIRE(!fails("const N = 3; struct S { n: u8[N]; x: u8; }", "declared later"));
    REQUIRE(fails("struct S { off: u8; at(off + later) { x: u8; } later: u8; }", "declared later"));
    std::cout << "Dependency validation tests passed\n";
}
