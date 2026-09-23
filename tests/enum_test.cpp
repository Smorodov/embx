#include <catch2/catch_test_macros.hpp>
#include "ir/IrBuilder.h"
#include "parser/ParserDriver.h"
#include "plan/PlanBuilder.h"
#include "semantic/SemanticAnalyzer.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

static std::unique_ptr<embx::plan::Module> build(const std::string& text, const char* path, std::string& err) {
    std::ofstream f(path); f << text; f.close();
    auto a = embx::parser::parseFile(path, err);
    if (!a || !err.empty()) { std::remove(path); return {}; }
    if (!embx::semantic::analyze(*a, err)) { std::remove(path); return {}; }
    auto ir = embx::ir::lower(*a, err);
    if (!ir || !err.empty()) { std::remove(path); return {}; }
    auto p = embx::plan::build(*ir, err);
    std::remove(path);
    return p;
}

static bool rejects(const std::string& text, const std::string& needle, int n) {
    std::string err;
    auto p = build(text, ("enum_negative_" + std::to_string(n) + ".embx").c_str(), err);
    return !p && err.find(needle) != std::string::npos;
}

TEST_CASE("enum", "[enum]") {
    std::string err;
    auto p = build(R"(
        const BASE = 10;
        enum E: u8 { A = BASE, B = A + 2, C = E::B + 1 }
        enum F: u8 { A = 7, B = A + 1 }
        struct S { x: u8; }
    )", "enum_test.embx", err);
    REQUIRE((p && err.empty()));
    REQUIRE(p->enums.size() == 2);
    REQUIRE(p->enums[0].values.at("A") == embx::runtime::Value(int64_t(10)));
    REQUIRE(p->enums[0].values.at("B") == embx::runtime::Value(int64_t(12)));
    REQUIRE(p->enums[0].values.at("C") == embx::runtime::Value(int64_t(13)));
    REQUIRE(p->enums[1].values.at("A") == embx::runtime::Value(int64_t(7)));

    REQUIRE(rejects("enum E: u8 { A = 256 } struct S { x: u8; }", "exceeds underlying type range", 1));
    REQUIRE(rejects("enum E: u8 { A = -1 } struct S { x: u8; }", "negative for unsigned", 2));
    REQUIRE(rejects("enum E: i8 { A = 128 } struct S { x: u8; }", "exceeds underlying type range", 3));
    REQUIRE(rejects("enum E: i8 { A = -129 } struct S { x: u8; }", "exceeds underlying type range", 4));
    REQUIRE(rejects("enum E { A = 4294967296 } struct S { x: u8; }", "exceeds underlying type range", 5));
    REQUIRE(rejects("enum E { A = -1 } struct S { x: u8; }", "negative for unsigned", 6));
    REQUIRE(rejects("enum E: u8 { A = 1, B = 1 } struct S { x: u8; }", "duplicate numeric value", 7));
    REQUIRE(rejects("enum E: u8 { A = 1.5 } struct S { x: u8; }", "integer expression", 8));
    REQUIRE(rejects("enum E: u8 { A = \"x\" } struct S { x: u8; }", "integer expression", 9));
    REQUIRE(rejects("enum E: u8 { A = 1, B = A == 1 } struct S { x: u8; }", "integer expression", 10));

    std::cout << "Enum tests passed\n";
}
