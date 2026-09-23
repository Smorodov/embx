#include <catch2/catch_test_macros.hpp>
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "reflection/Reflection.h"
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>

namespace {
std::unique_ptr<embx::plan::Module> makePlan(const std::string& source, std::string& err) {
    const auto path = std::filesystem::temp_directory_path() / "embx_variant_closure_test.embx";
    { std::ofstream out(path); out << source; }
    auto ast = embx::parser::parseFile(path.string(), err);
    if (!ast || !err.empty() || !embx::semantic::analyze(*ast, err) || !err.empty()) { std::filesystem::remove(path); return {}; }
    auto ir = embx::ir::lower(*ast, err);
    if (!ir || !err.empty()) { std::filesystem::remove(path); return {}; }
    auto plan = embx::plan::build(*ir, err);
    std::filesystem::remove(path);
    return plan;
}
}

TEST_CASE("variant materializes compile-time tags and executes explicit/default cases") {
    std::string err;
    auto plan = makePlan(R"(const ONE = 1;
const TWO = 2;
struct Packet {
  kind: u8;
  variant body by kind {
    ONE: u16;
    TWO: { value: u32; }
    default: u8;
  }
})", err);
    REQUIRE(plan);
    REQUIRE(err.empty());
    const auto* v = dynamic_cast<const embx::plan::Variant*>(plan->structs[0].members[1].get());
    REQUIRE(v);
    REQUIRE(v->name == "body");
    REQUIRE(v->discriminator);
    REQUIRE(v->discriminator->reference.valid());
    REQUIRE(v->cases.size() == 2);
    REQUIRE(std::get<std::uint64_t>(v->cases[0].tag) == 1);
    REQUIRE(std::get<std::uint64_t>(v->cases[1].tag) == 2);
    REQUIRE(v->hasDefault);

    const auto reflected = embx::reflection::inspectStruct(*plan, "Packet");
    REQUIRE(reflected.has_value());
    REQUIRE(reflected->members[1].discriminator == "kind");
    REQUIRE(reflected->members[1].children == std::vector<std::string>{"1", "2", "default"});

    embx::encoder::Engine encoder(*plan);
    embx::value::Value::Object one;
    one["kind"] = std::uint64_t(1);
    embx::value::Value::Object oneBody;
    oneBody["value"] = std::uint64_t(0x3344);
    one["body"] = embx::value::Value(oneBody);
    auto e1 = encoder.encode("Packet", embx::value::Value(one));
    REQUIRE(e1.success);
    REQUIRE(e1.data == std::vector<std::uint8_t>{1, 0x44, 0x33});

    embx::decoder::Engine decoder(*plan);
    auto d1 = decoder.decode("Packet", e1.data);
    REQUIRE(d1.success);
    REQUIRE(std::get<std::uint64_t>(d1.value.at("kind").data) == 1);
    auto* body1 = d1.value.get("body");
    REQUIRE(body1);
    auto* obj1 = std::get_if<embx::value::Value::Object>(&body1->data);
    REQUIRE(obj1);
    REQUIRE(std::get<std::uint64_t>(obj1->at("value").data) == 0x3344);

    embx::value::Value::Object def;
    def["kind"] = std::uint64_t(9);
    embx::value::Value::Object defBody;
    defBody["value"] = std::uint64_t(7);
    def["body"] = embx::value::Value(defBody);
    auto ed = encoder.encode("Packet", embx::value::Value(def));
    REQUIRE(ed.success);
    REQUIRE(ed.data == std::vector<std::uint8_t>{9, 7});
    auto dd = decoder.decode("Packet", ed.data);
    REQUIRE(dd.success);
    REQUIRE(std::get<std::uint64_t>(dd.value.at("kind").data) == 9);
}

TEST_CASE("variant rejects runtime-dependent and duplicate semantic tags") {
    std::string err;
    auto plan = makePlan(R"(struct S {
  kind: u8;
  value: u8;
  variant v by kind { value: u16; }
})", err);
    REQUIRE_FALSE(plan);
    REQUIRE(err.find("compile-time constant") != std::string::npos);

    err.clear();
    plan = makePlan(R"(const A = 1;
const B = 1;
struct S {
  kind: u8;
  variant v by kind { A: u8; B: u16; }
})", err);
    REQUIRE_FALSE(plan);
    REQUIRE(err.find("duplicate variant tag") != std::string::npos);
}

TEST_CASE("variant rejects non-integer discriminator and out-of-range tags") {
    std::string err;
    auto plan = makePlan(R"(struct S {
  name: string[4];
  variant v by name { 1: u8; }
})", err);
    REQUIRE_FALSE(plan);
    REQUIRE(err.find("discriminator must be an integer") != std::string::npos);

    err.clear();
    plan = makePlan(R"(const BAD = 18446744073709551615;
struct S {
  kind: i64;
  variant v by kind { BAD: u8; }
})", err);
    REQUIRE_FALSE(plan);
    REQUIRE(err.find("out of range") != std::string::npos);
}

TEST_CASE("nested variant and conditional execute only the selected branch") {
    std::string err;
    auto plan = makePlan(R"(struct Packet {
  kind: u8;
  flag: u8;
  variant body by kind {
    1: {
      if (flag == 1) { yes: u16; }
      else { no: u8; }
    }
    default: {
      fallback: u8;
    }
  }
})", err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    embx::value::Value::Object yesBody;
    yesBody["yes"] = uint64_t(0x1234);
    embx::value::Value::Object yes;
    yes["kind"] = uint64_t(1);
    yes["flag"] = uint64_t(1);
    yes["body"] = embx::value::Value(yesBody);
    auto encodedYes = embx::encoder::Engine(*plan).encode("Packet", embx::value::Value(yes));
    REQUIRE(encodedYes.success);
    REQUIRE(encodedYes.data == std::vector<uint8_t>{1, 1, 0x34, 0x12});

    auto decodedYes = embx::decoder::Engine(*plan).decode("Packet", encodedYes.data);
    REQUIRE(decodedYes.success);
    auto* body = decodedYes.value.get("body");
    REQUIRE(body);
    auto* bodyObject = std::get_if<embx::value::Value::Object>(&body->data);
    REQUIRE(bodyObject);
    REQUIRE(std::get<uint64_t>(bodyObject->at("yes").data) == 0x1234);
    REQUIRE(bodyObject->find("no") == bodyObject->end());
    REQUIRE(bodyObject->find("fallback") == bodyObject->end());

    embx::value::Value::Object fallbackBody;
    fallbackBody["fallback"] = uint64_t(7);
    embx::value::Value::Object fallback;
    fallback["kind"] = uint64_t(9);
    fallback["flag"] = uint64_t(1);
    fallback["body"] = embx::value::Value(fallbackBody);
    auto encodedFallback = embx::encoder::Engine(*plan).encode("Packet", embx::value::Value(fallback));
    REQUIRE(encodedFallback.success);
    REQUIRE(encodedFallback.data == std::vector<uint8_t>{9, 1, 7});
}
