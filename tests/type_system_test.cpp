#include <catch2/catch_test_macros.hpp>
#include "ir/IrBuilder.h"
#include "parser/ParserDriver.h"
#include "plan/PlanBuilder.h"
#include "semantic/SemanticAnalyzer.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace {
std::unique_ptr<embx::ast::Module> parse(const std::string& text, const std::string& name, std::string& err) {
    std::ofstream f(name); f << text; f.close();
    auto m = embx::parser::parseFile(name, err);
    std::remove(name.c_str());
    return m;
}

bool accepts(const char* text, int n) {
    std::string err;
    auto m = parse(text, "type_system_" + std::to_string(n) + ".embx", err);
    if (!m || !err.empty() || !embx::semantic::analyze(*m, err)) return false;
    auto ir = embx::ir::lower(*m, err);
    if (!ir || !err.empty()) return false;
    auto p = embx::plan::build(*ir, err);
    return p && err.empty();
}

bool rejects(const char* text, const char* needle, int n) {
    std::string err;
    auto m = parse(text, "type_system_negative_" + std::to_string(n) + ".embx", err);
    if (!m || !err.empty()) return false;
    if (!embx::semantic::analyze(*m, err)) return err.find(needle) != std::string::npos;
    auto ir = embx::ir::lower(*m, err);
    if (!ir || !err.empty()) return err.find(needle) != std::string::npos;
    auto p = embx::plan::build(*ir, err);
    return !p && err.find(needle) != std::string::npos;
}
}

TEST_CASE("type system", "[type_system]") {
    REQUIRE(accepts("type Word = u16; type Word2 = Word; struct S { x: Word2; }", 1));
    REQUIRE(accepts("type Word = u16; enum E: Word { A = 1, B = 2 }", 2));
    REQUIRE(accepts("type Word = u16; struct S { bits { x: Word (8); } }", 3));
    REQUIRE(accepts("type Payload = bytes[*]; type Payload2 = Payload; struct S { block body[4] { x: Payload2; } }", 4));
    REQUIRE(accepts("type Word = u16; type Word2 = Word; enum E: Word2 { A = 1 }", 5));

    REQUIRE(rejects("const N = 1; type X = N;", "unknown type: N", 6));
    REQUIRE(rejects("type A = B; type B = A;", "cyclic type alias", 7));
    REQUIRE(rejects("type Word = u16[2]; enum E: Word { A = 1 }", "scalar integer", 8));
    REQUIRE(rejects("type Bytes = bytes; struct S { bits { x: Bytes (8); } }", "scalar integer", 9));
    REQUIRE(rejects("type Float = f32; enum E: Float { A = 1 }", "scalar integer", 10));
    REQUIRE(rejects("type X = Missing;", "unknown type: Missing", 11));
    REQUIRE(rejects("type B = bytes[4]; struct S { x: B [5]; }", "array/length may be specified either", 12));

    REQUIRE(rejects("enum E: u8 { A = 256 } struct S { x: u8; }", "exceeds underlying type range", 13));
    REQUIRE(rejects("enum E: u8 { A = -1 } struct S { x: u8; }", "negative for unsigned", 14));
    REQUIRE(rejects("enum E: i8 { A = 128 } struct S { x: u8; }", "exceeds underlying type range", 15));
    REQUIRE(rejects("enum E: i8 { A = -129 } struct S { x: u8; }", "exceeds underlying type range", 16));
    REQUIRE(rejects("enum E: u8 { A = 1, B = 1 } struct S { x: u8; }", "duplicate numeric value", 17));
    REQUIRE(rejects("const Bad = \"x\" + 1;", "arithmetic operator requires numeric operands", 22));
    REQUIRE(rejects("const Bad = 1 && 2;", "logical operator requires boolean operands", 23));
    REQUIRE(rejects("const Bad = 1.0 % 2;", "% requires integer operands", 24));
    REQUIRE(rejects("enum E: u8 { A = 1.5 } struct S { x: u8; }", "integer expression", 18));
    REQUIRE(rejects("enum E: u8 { A = \"x\" } struct S { x: u8; }", "integer expression", 19));
    REQUIRE(accepts("enum E: u8 { A = 1, B = A + 2 } struct S { x: u8; }", 20));
    REQUIRE(accepts("enum E: u8 { A = 1 } struct S { x: u8[A]; }", 21));
    std::cout << "Type system tests passed\n";
}
