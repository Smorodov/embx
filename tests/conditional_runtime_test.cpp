#include <catch2/catch_test_macros.hpp>
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace {
std::shared_ptr<embx::plan::Module> makePlan(const std::string& source, std::string& err) {
    const auto path = std::filesystem::temp_directory_path() / "embx_conditional_runtime.embx";
    std::ofstream out(path);
    out << source;
    out.close();
    auto ast = embx::parser::parseFile(path.string(), err);
    if (!ast || !err.empty() || !embx::semantic::analyze(*ast, err) || !err.empty()) {
        std::filesystem::remove(path);
        return {};
    }
    auto ir = embx::ir::lower(*ast, err);
    if (!ir || !err.empty()) {
        std::filesystem::remove(path);
        return {};
    }
    auto plan = embx::plan::build(*ir, err);
    std::filesystem::remove(path);
    return plan;
}
}


TEST_CASE("conditional accepts boolean literals", "[conditional]") {
    std::string err;
    auto plan = makePlan(R"(struct Packet {
  if (true) { value: u8; }
})", err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    embx::value::Value::Object object;
    object["value"] = uint64_t(7);
    auto encoded = embx::encoder::Engine(*plan).encode("Packet", embx::value::Value(object));
    REQUIRE(encoded.success);
    REQUIRE(encoded.data == std::vector<uint8_t>{7});
}

TEST_CASE("conditional fields encode and decode the selected branch", "[conditional]") {
    std::string err;
    auto plan = makePlan(R"(struct Packet {
  flags: u8;
  if (flags == 1) { value: u32; }
  else { small: u16; }
})", err);

    embx::value::Value::Object thenObject;
    thenObject["flags"] = uint64_t(1);
    thenObject["value"] = uint64_t(0x11223344);
    embx::encoder::Engine encoder(*plan);
    auto encodedThen = encoder.encode("Packet", embx::value::Value(thenObject));
    REQUIRE(encodedThen.success);
    REQUIRE(encodedThen.data == std::vector<uint8_t>{0x01, 0x44, 0x33, 0x22, 0x11});

    embx::decoder::Engine decoder(*plan);
    auto decodedThen = decoder.decode("Packet", encodedThen.data);
    REQUIRE(decodedThen.success);
    REQUIRE(decodedThen.consumed == encodedThen.data.size());
    REQUIRE(decodedThen.value.get("value"));
    REQUIRE(std::get<uint64_t>(decodedThen.value.at("value").data) == 0x11223344);
    REQUIRE(decodedThen.value.get("small") == nullptr);

    embx::value::Value::Object elseObject;
    elseObject["flags"] = uint64_t(0);
    elseObject["small"] = uint64_t(0x5566);
    auto encodedElse = encoder.encode("Packet", embx::value::Value(elseObject));
    REQUIRE(encodedElse.success);
    REQUIRE(encodedElse.data == std::vector<uint8_t>{0x00, 0x66, 0x55});

    auto decodedElse = decoder.decode("Packet", encodedElse.data);
    REQUIRE(decodedElse.success);
    REQUIRE(decodedElse.consumed == encodedElse.data.size());
    REQUIRE(decodedElse.value.get("small"));
    REQUIRE(std::get<uint64_t>(decodedElse.value.at("small").data) == 0x5566);
    REQUIRE(decodedElse.value.get("value") == nullptr);
}

TEST_CASE("conditional fields can depend on runtime parameters", "[conditional]") {
    std::string err;
    auto plan = makePlan(R"(param enabled: u8;
struct Packet {
  value: u8;
  if (enabled == 1) { extra: u16; }
  else { fallback: u8; }
})", err);
    const auto enabled = plan->symbolTable.findId("enabled");
    REQUIRE(enabled != embx::core::InvalidSymbolId);
    REQUIRE(err.empty());

    embx::encoder::Options on;
    on.parameters[enabled] = uint64_t(1);
    embx::value::Value::Object onObject;
    onObject["value"] = uint64_t(7);
    onObject["extra"] = uint64_t(0x1234);
    auto encodedOn = embx::encoder::Engine(*plan, on).encode("Packet", embx::value::Value(onObject));
    REQUIRE(encodedOn.success);
    REQUIRE(encodedOn.data == std::vector<uint8_t>{7, 0x34, 0x12});

    embx::encoder::Options offEncode;
    offEncode.parameters[enabled] = uint64_t(0);
    embx::value::Value::Object offObject;
    offObject["value"] = uint64_t(7);
    offObject["fallback"] = uint64_t(9);
    auto encodedOff = embx::encoder::Engine(*plan, offEncode).encode("Packet", embx::value::Value(offObject));
    REQUIRE(encodedOff.success);
    REQUIRE(encodedOff.data == std::vector<uint8_t>{7, 9});

    embx::decoder::Options off;
    off.parameters[enabled] = uint64_t(0);
    auto decodedOff = embx::decoder::Engine(*plan, off).decode("Packet", encodedOff.data);
    REQUIRE(decodedOff.success);
    REQUIRE(decodedOff.value.get("fallback"));
    REQUIRE(std::get<uint64_t>(decodedOff.value.at("fallback").data) == 9);
    REQUIRE(decodedOff.value.get("extra") == nullptr);
}

TEST_CASE("nested conditionals isolate branch-local fields") {
    std::string err;
    auto plan = makePlan(R"(struct Packet {
  flag: u8;
  if (flag == 1) {
    outer: u8;
    if (outer == 2) { deep: u16; }
    else { shallow: u8; }
  } else {
    fallback: u8;
  }
})", err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    embx::value::Value::Object deep;
    deep["flag"] = uint64_t(1);
    deep["outer"] = uint64_t(2);
    deep["deep"] = uint64_t(0x1234);
    auto encodedDeep = embx::encoder::Engine(*plan).encode("Packet", embx::value::Value(deep));
    REQUIRE(encodedDeep.success);
    REQUIRE(encodedDeep.data == std::vector<uint8_t>{1, 2, 0x34, 0x12});

    auto decodedDeep = embx::decoder::Engine(*plan).decode("Packet", encodedDeep.data);
    REQUIRE(decodedDeep.success);
    REQUIRE(decodedDeep.value.get("deep"));
    REQUIRE(decodedDeep.value.get("shallow") == nullptr);
    REQUIRE(decodedDeep.value.get("fallback") == nullptr);

    embx::value::Value::Object fallback;
    fallback["flag"] = uint64_t(0);
    fallback["fallback"] = uint64_t(9);
    auto encodedFallback = embx::encoder::Engine(*plan).encode("Packet", embx::value::Value(fallback));
    REQUIRE(encodedFallback.success);
    REQUIRE(encodedFallback.data == std::vector<uint8_t>{0, 9});
    auto decodedFallback = embx::decoder::Engine(*plan).decode("Packet", encodedFallback.data);
    REQUIRE(decodedFallback.success);
    REQUIRE(decodedFallback.value.get("fallback"));
    REQUIRE(decodedFallback.value.get("outer") == nullptr);
    REQUIRE(decodedFallback.value.get("deep") == nullptr);
}
