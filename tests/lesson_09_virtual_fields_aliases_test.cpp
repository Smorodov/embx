#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "reflection/Reflection.h"

namespace {
std::shared_ptr<embx::plan::Module> makePlan(const std::string& source, std::string& err) {
    const std::string path = "lesson_09_virtual_fields_aliases.embx";
    { std::ofstream out(path); out << source; }
    auto ast = embx::parser::parseFile(path, err);
    std::remove(path.c_str());
    if (!ast) return {};
    if (!embx::semantic::analyze(*ast, err)) return {};
    auto ir = embx::ir::lower(*ast, err);
    if (!ir) return {};
    auto plan = embx::plan::build(*ir, err);
    if (!plan) return {};
    return std::shared_ptr<embx::plan::Module>(std::move(plan));
}
}

TEST_CASE("virtual field and alias add no physical bytes") {
    std::string err;
    auto plan = makePlan(R"(struct Projected big {
  value: u8;
  let doubled = value * 2;
  alias value2 = value;
})", err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    embx::value::Value::Object input;
    input["value"] = uint64_t(7);
    embx::encoder::Engine encoder(*plan);
    auto encoded = encoder.encode("Projected", input);
    REQUIRE(encoded);
    REQUIRE(encoded.data == std::vector<uint8_t>{7});

    embx::decoder::Engine decoder(*plan);
    auto decoded = decoder.decode("Projected", encoded.data);
    REQUIRE(decoded);
    REQUIRE(std::get<uint64_t>(decoded.value.at("value").data) == 7);
    REQUIRE(std::get<uint64_t>(decoded.value.at("doubled").data) == 14);
    REQUIRE(std::get<uint64_t>(decoded.value.at("value2").data) == 7);
}

TEST_CASE("virtual field and alias can feed later expressions") {
    std::string err;
    auto plan = makePlan(R"(struct Projected big {
  value: u8;
  let doubled = value * 2;
  alias length = value;
  payload: bytes[doubled];
  tail: bytes[length];
})", err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    embx::value::Value::Object input;
    input["value"] = uint64_t(2);
    input["payload"] = embx::value::Value::Bytes{1, 2, 3, 4};
    input["tail"] = embx::value::Value::Bytes{5, 6};
    embx::encoder::Engine encoder(*plan);
    auto encoded = encoder.encode("Projected", input);
    REQUIRE(encoded);
    REQUIRE(encoded.data == std::vector<uint8_t>{2, 1, 2, 3, 4, 5, 6});

    embx::decoder::Engine decoder(*plan);
    auto decoded = decoder.decode("Projected", encoded.data);
    REQUIRE(decoded);
    REQUIRE(std::get<uint64_t>(decoded.value.at("length").data) == 2);
    REQUIRE(decoded.value.get("payload"));
    REQUIRE(decoded.value.get("tail"));
}

TEST_CASE("alias preserves the physical target in reflection") {
    std::string err;
    auto plan = makePlan(R"(struct Projected {
  value: u8;
  let doubled = value * 2;
  alias value2 = value;
})", err);
    REQUIRE(plan);
    auto reflected = embx::reflection::inspectStruct(*plan, "Projected");
    REQUIRE(reflected);
    REQUIRE(reflected->members.size() == 3);
    REQUIRE(reflected->members[1].kind == embx::reflection::Kind::Virtual);
    REQUIRE(reflected->members[2].kind == embx::reflection::Kind::Alias);
    REQUIRE(reflected->members[2].name == "value2");
    REQUIRE(reflected->members[2].children == std::vector<std::string>{"value"});
}
