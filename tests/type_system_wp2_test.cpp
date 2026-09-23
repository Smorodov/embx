#include <catch2/catch_test_macros.hpp>
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

static bool analyzeOk(const char* src, int n, std::string* out=nullptr) {
    const std::string path = "wp2_ok_" + std::to_string(n) + ".embx";
    { std::ofstream f(path); f << src; }
    std::string err;
    auto m = embx::parser::parseFile(path, err);
    bool ok = m && embx::semantic::analyze(*m, err);
    if (out) *out = err;
    std::remove(path.c_str());
    return ok;
}

static bool analyzeBad(const char* src, const char* needle, int n) {
    std::string err;
    const bool parsed = analyzeOk(src, n, &err);
    return !parsed && err.find(needle) != std::string::npos;
}

TEST_CASE("type system wp2", "[type_system_wp2]") {
    // Alias chains are valid and may terminate in any named executable type.
    REQUIRE(analyzeOk("type A = u16; type B = A; struct S { x: B; }", 1));
    REQUIRE(analyzeOk("struct Node { next: Node; }", 2));
    REQUIRE(analyzeOk("struct Node { next: Link; } type Link = Node;", 3));

    // Enum underlying types may now use an integer alias chain, but the
    // resolved target must remain a scalar integer.
    REQUIRE(analyzeOk("type U = u16; type V = U; enum E: V { A = 1 }", 4));
    REQUIRE(analyzeBad("type A = B; type B = A;", "cyclic type alias", 5));
    REQUIRE(analyzeBad("type A = B[2]; type B = A;", "cyclic type alias", 6));
    REQUIRE(analyzeBad("type U = bytes; enum E: U { A = 1 }", "enum underlying type must resolve to a scalar integer", 7));
    REQUIRE(analyzeBad("type U = u16[2]; enum E: U { A = 1 }", "enum underlying type must be a scalar integer", 8));

    // Deterministic named-symbol collision rule: all top-level declarations
    // occupy one namespace; primitives are reserved and cannot be shadowed.
    REQUIRE(analyzeBad("type X = u8; struct X { a: u8; }", "duplicate symbol: X", 9));

    // Provably invalid expression categories are rejected before Plan build.
    REQUIRE(analyzeBad("struct S { a: u8[1.5]; }", "array length must be an integer expression", 10));
    REQUIRE(analyzeBad("struct S { block b[\"x\"] { a: u8; } }", "block size must be an integer expression", 11));
    REQUIRE(analyzeBad("struct S { at(2.5) { a: u8; } }", "at offset must be an integer expression", 12));
    REQUIRE(analyzeBad("struct S { align(1.5); }", "alignment must be an integer expression", 13));

    std::cout << "WP2 type-system tests passed\n";
}
