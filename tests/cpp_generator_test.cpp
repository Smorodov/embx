#include <catch2/catch_test_macros.hpp>
#include "codegen/CppGenerator.h"
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
#include <string>

static embx::codegen::Output generate(const std::string& path) {
    std::string error;
    auto ast = embx::parser::parseFile(path, error);
    REQUIRE(ast);
    REQUIRE(error.empty());
    REQUIRE(embx::semantic::analyze(*ast, error));
    REQUIRE(error.empty());
    auto ir = embx::ir::lower(*ast, error);
    REQUIRE(ir);
    REQUIRE(error.empty());
    auto plan = embx::plan::build(*ir, error);
    REQUIRE(plan);
    REQUIRE(error.empty());
    embx::codegen::Output out;
    REQUIRE(embx::codegen::generateCpp(*plan, out, error));
    REQUIRE(error.empty());
    return out;
}

TEST_CASE("generator emits stable schema types and constants") {
    const auto out = generate("../examples/common.embx");
    REQUIRE(out.header.find("std::uint16_t magic{};") != std::string::npos);
    REQUIRE(out.header.find("std::uint8_t kind{};") != std::string::npos);
    REQUIRE(out.header.find("inline constexpr std::uint64_t MAGIC = 41394ULL;") != std::string::npos);
    REQUIRE(out.source.find("generated.hpp") != std::string::npos);
}

TEST_CASE("generator supports dynamic layout operations") {
    std::string error;
    auto ast = embx::parser::parseFile("../examples/container.embx", error);
    REQUIRE(ast);
    REQUIRE(embx::semantic::analyze(*ast, error));
    auto ir = embx::ir::lower(*ast, error);
    REQUIRE(ir);
    auto plan = embx::plan::build(*ir, error);
    REQUIRE(plan);
    embx::codegen::Output out;
    REQUIRE(embx::codegen::generateCpp(*plan, out, error));
    REQUIRE(error.empty());
    REQUIRE(out.header.find("struct Container_payload") != std::string::npos);
    REQUIRE(out.header.find("std::vector<std::uint8_t> data{};") != std::string::npos);
    REQUIRE(out.source.find("checkedHostSize(blockN64,blockN)") != std::string::npos);
    REQUIRE(out.source.find("block size mismatch") != std::string::npos);
}
