#include <catch2/catch_test_macros.hpp>
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include <cstdio>
#include <fstream>
#include <string>

TEST_CASE("semantic type resolution accepts canonical named types and rejects non-types") {
    const std::string path = "semantic_type_resolution_contract.embx";
    {
        std::ofstream out(path);
        out << "type Word = u16; struct Packet { value: Word; }";
    }
    std::string err;
    auto module = embx::parser::parseFile(path, err);
    std::remove(path.c_str());
    REQUIRE(module);
    REQUIRE(err.empty());
    REQUIRE(embx::semantic::analyze(*module, err));

    const std::string badPath = "semantic_type_resolution_non_type.embx";
    {
        std::ofstream out(badPath);
        out << "const N = 1; struct Packet { value: N; }";
    }
    err.clear();
    auto bad = embx::parser::parseFile(badPath, err);
    std::remove(badPath.c_str());
    REQUIRE(bad);
    REQUIRE(err.empty());
    REQUIRE_FALSE(embx::semantic::analyze(*bad, err));
    REQUIRE(err.find("unknown type: N") != std::string::npos);
}
