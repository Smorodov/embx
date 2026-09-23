#include <catch2/catch_test_macros.hpp>
#include "parser/ParserDriver.h"
#include "ir/IrBuilder.h"
#include "semantic/SemanticAnalyzer.h"
#include "plan/PlanBuilder.h"
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"
#include "reflection/Reflection.h"
#include "codegen/CppGenerator.h"
#include <cstdint>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

namespace {
std::unique_ptr<embx::plan::Module> makePlan(const std::string& source) {
    std::string err;
    const auto path = std::filesystem::temp_directory_path() / "embx_requires_test.embx";
    { std::ofstream out(path); out << source; }
    auto ast = embx::parser::parseFile(path.string(), err);
    REQUIRE(ast);
    REQUIRE(err.empty());
    REQUIRE(embx::semantic::analyze(*ast, err));
    REQUIRE(err.empty());
    auto ir = embx::ir::lower(*ast, err);
    std::filesystem::remove(path);
    REQUIRE(ir);
    auto plan = embx::plan::build(*ir, err);
    REQUIRE(plan);
    return plan;
}
embx::value::Value u(std::uint64_t x) { return embx::value::Value(x); }
}

TEST_CASE("requires is canonical plan constraint and symmetric at runtime") {
    const auto plan = makePlan(R"(
struct Packet {
  value: u16;
  requires value == 7;
}
)");

    const auto reflected = embx::reflection::inspectStruct(*plan, "Packet");
    REQUIRE(reflected.has_value());
    REQUIRE(reflected->requirements == std::vector<std::string>{"value==7"});

    embx::codegen::Output generated;
    std::string generationError;
    REQUIRE(embx::codegen::generateCpp(*plan, generated, generationError));
    REQUIRE(generated.source.find("requires constraint failed") != std::string::npos);

    embx::decoder::Engine decoder(*plan);
    auto good = decoder.decode("Packet", {7, 0});
    REQUIRE(good.success);
    auto bad = decoder.decode("Packet", {8, 0});
    REQUIRE_FALSE(bad.success);
    REQUIRE(bad.error.find("requires constraint failed") != std::string::npos);

    embx::encoder::Engine encoder(*plan);
    embx::value::Value::Object goodObject;
    goodObject.emplace("value", u(7));
    auto encoded = encoder.encode("Packet", embx::value::Value(std::move(goodObject)));
    REQUIRE(encoded.success);
    REQUIRE(encoded.error.empty());
    REQUIRE(encoded.data == std::vector<std::uint8_t>{7, 0});

    embx::value::Value::Object badObject;
    badObject.emplace("value", u(8));
    auto rejected = encoder.encode("Packet", embx::value::Value(std::move(badObject)));
    REQUIRE_FALSE(rejected.success);
    REQUIRE(rejected.error.find("requires constraint failed") != std::string::npos);
}

TEST_CASE("requires can consume Plan materialized module values by SymbolId") {
    const auto plan = makePlan(R"(
const LIMIT = 7;
enum Mode: u8 { Good = 1 }
struct Packet {
  value: u16;
  mode: u8;
  requires value == LIMIT;
  requires mode == Mode::Good;
}
)");

    embx::value::Value::Object goodObject;
    goodObject.emplace("value", u(7));
    goodObject.emplace("mode", u(1));
    embx::encoder::Engine encoder(*plan);
    auto good = encoder.encode("Packet", embx::value::Value(goodObject));
    REQUIRE(good.success);

    embx::decoder::Engine decoder(*plan);
    auto decoded = decoder.decode("Packet", good.data);
    REQUIRE(decoded.success);

    goodObject["value"] = u(8);
    auto bad = encoder.encode("Packet", embx::value::Value(goodObject));
    REQUIRE_FALSE(bad.success);
    REQUIRE(bad.error.find("requires constraint failed") != std::string::npos);
}

TEST_CASE("requires rejects non-boolean expressions and unresolved dependencies") {
    std::string err;
    const auto path = std::filesystem::temp_directory_path() / "embx_requires_negative_test.embx";
    { std::ofstream out(path); out << "struct S { value: u16; requires value + 1; }"; }
    auto ast = embx::parser::parseFile(path.string(), err);
    REQUIRE(ast);
    REQUIRE(err.empty());
    REQUIRE(embx::semantic::analyze(*ast, err));
    REQUIRE(err.empty());
    auto ir = embx::ir::lower(*ast, err);
    REQUIRE(ir);
    auto plan = embx::plan::build(*ir, err);
    REQUIRE_FALSE(plan);
    REQUIRE(err.find("requires expression must be boolean") != std::string::npos);
    std::filesystem::remove(path);

    err.clear();
    { std::ofstream out(path); out << "struct S { value: u16; requires missing == 7; }"; }
    ast = embx::parser::parseFile(path.string(), err);
    REQUIRE(ast);
    REQUIRE(err.empty());
    REQUIRE(embx::semantic::analyze(*ast, err));
    REQUIRE(err.empty());
    ir = embx::ir::lower(*ast, err);
    REQUIRE(ir);
    plan = embx::plan::build(*ir, err);
    REQUIRE_FALSE(plan);
    REQUIRE(err.find("unresolved identifier") != std::string::npos);
    std::filesystem::remove(path);
}
