#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"

namespace {
std::shared_ptr<embx::plan::Module> makePlan(const std::string& source, std::string& err) {
    const std::string path = "virtual_field_test.embx";
    { std::ofstream out(path); out << source; }
    auto ast = embx::parser::parseFile(path, err);
    std::remove(path.c_str());
    if (!ast) return {};
    auto sem = embx::semantic::analyze(*ast, err);
    if (!sem) return {};
    auto ir = embx::ir::lower(*ast, err);
    if (!ir) return {};
    auto plan = embx::plan::build(*ir, err);
    if (!plan) return {};
    return std::shared_ptr<embx::plan::Module>(std::move(plan));
}
}

TEST_CASE("virtual field is executable and consumes no bytes") {
    std::string err;
    auto plan = makePlan(R"(struct Packet {
  value: u8;
  let doubled = value * 2;
})", err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    embx::value::Value::Object input;
    input["value"] = uint64_t(7);
    embx::encoder::Engine encoder(*plan);
    auto encoded = encoder.encode("Packet", input);
    REQUIRE(encoded);
    REQUIRE(encoded.data == std::vector<uint8_t>{7});

    embx::decoder::Engine decoder(*plan);
    auto decoded = decoder.decode("Packet", encoded.data);
    REQUIRE(decoded);
    REQUIRE(decoded.value.get("value"));
    REQUIRE(std::get<uint64_t>(decoded.value.at("value").data) == 7);
    REQUIRE(decoded.value.get("doubled"));
    REQUIRE(std::get<uint64_t>(decoded.value.at("doubled").data) == 14);
}

TEST_CASE("virtual field can drive a later layout expression") {
    std::string err;
    auto plan = makePlan(R"(struct Packet {
  count: u8;
  let bytes_count = count * 2;
  payload: bytes[bytes_count];
})", err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    embx::value::Value::Object input;
    input["count"] = uint64_t(2);
    input["payload"] = embx::value::Value::Bytes{1,2,3,4};
    embx::encoder::Engine encoder(*plan);
    auto encoded = encoder.encode("Packet", input);
    REQUIRE(encoded);
    REQUIRE(encoded.data == std::vector<uint8_t>{2,1,2,3,4});

    embx::decoder::Engine decoder(*plan);
    auto decoded = decoder.decode("Packet", encoded.data);
    REQUIRE(decoded);
    REQUIRE(decoded.value.get("bytes_count"));
    REQUIRE(std::get<uint64_t>(decoded.value.at("bytes_count").data) == 4);
}

TEST_CASE("virtual field cannot reference a later member") {
    std::string err;
    auto plan = makePlan(R"(struct Packet {
  let doubled = value * 2;
  value: u8;
})", err);
    REQUIRE_FALSE(plan);
    const bool dependencyError =
        err.find("unavailable") != std::string::npos ||
        err.find("not yet") != std::string::npos ||
        err.find("dependency") != std::string::npos;
    REQUIRE(dependencyError);
}
