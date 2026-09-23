#include <catch2/catch_test_macros.hpp>
#include "ir/IrBuilder.h"
#include "parser/ParserDriver.h"
#include "plan/PlanBuilder.h"
#include "semantic/SemanticAnalyzer.h"
#include <cstdio>
#include <fstream>
#include <iostream>

static bool buildFails(const char* text, const char* needle){
    const char* path="plan_negative.embx"; std::ofstream g(path);g<<text;g.close();std::string err;auto a=embx::parser::parseFile(path,err);if(!a){std::remove(path);return true;}if(!embx::semantic::analyze(*a,err)){std::remove(path);return true;}auto ir=embx::ir::lower(*a,err);if(!ir){std::remove(path);return true;}auto p=embx::plan::build(*ir,err);std::remove(path);return !p && err.find(needle)!=std::string::npos;
}
TEST_CASE("plan negative", "[plan_negative]") {
 REQUIRE(buildFails("const A = B; const B = A; struct S { x: u8; }","dependency cycle"));
 REQUIRE(buildFails("struct S { x: bytes[*]; }","bounded context"));
 REQUIRE(buildFails("struct S { align(0); }","alignment cannot be zero"));
 REQUIRE(buildFails("struct S { flag: u8; if (flag && true) { value: u8; } }","logical operator requires boolean operands"));
 REQUIRE(buildFails("struct S { text: string[4]; if (text == 1) { value: u8; } }","comparison operands must be numeric"));
 REQUIRE(buildFails("struct S { flag: string[1]; x: u8[-flag]; }","unary '-' requires a numeric operand"));
 REQUIRE(buildFails("struct S { flag: u8; block b[flag && true] { value: u8; } }","logical operator requires boolean operands"));
 std::cout<<"Plan negative tests passed\n";
}
