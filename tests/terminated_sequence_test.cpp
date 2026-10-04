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
    const std::string path = "terminated_sequence_test.embx";
    { std::ofstream out(path); out << source; }
    auto ast = embx::parser::parseFile(path, err);
    std::remove(path.c_str());
    if (!ast || !embx::semantic::analyze(*ast, err)) return {};
    auto ir = embx::ir::lower(*ast, err);
    if (!ir) return {};
    auto plan = embx::plan::build(*ir, err);
    if (!plan) return {};
    return std::shared_ptr<embx::plan::Module>(std::move(plan));
}
}

TEST_CASE("terminated byte sequence decodes, advances past terminator, and re-encodes") {
    std::string err;
    auto plan = makePlan(R"(struct Record {
  name: bytes until 0x00 max 8;
  tail: u8;
})", err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    const auto sid = plan->symbolTable.findId("Record");
    REQUIRE(sid != embx::core::InvalidSymbolId);
    const auto& s = plan->structs.at(plan->structIndexBySymbol.at(sid));
    REQUIRE(s.members.size() == 2);
    const auto& field = static_cast<const embx::plan::Field&>(*s.members[0]);
    REQUIRE(field.type.terminator == std::vector<std::uint8_t>{0x00});
    REQUIRE(field.type.maxPayload == 8);

    const auto bounds = embx::plan::layoutBounds(*plan, field.type, err);
    REQUIRE(err.empty());
    REQUIRE(bounds.minSize == 1);
    REQUIRE(bounds.maxSize == 9);
    REQUIRE(bounds.classification() == embx::plan::LayoutClass::Bounded);

    embx::decoder::Engine decoder(*plan);
    const std::vector<std::uint8_t> wire{'H','i',0x00,0x7f};
    const auto decoded = decoder.decode("Record", wire);
    REQUIRE(decoded);
    REQUIRE(decoded.consumed == wire.size());
    REQUIRE(std::get<embx::value::Value::Bytes>(decoded.value.at("name").data) == embx::value::Value::Bytes{'H','i'});
    REQUIRE(std::get<std::uint64_t>(decoded.value.at("tail").data) == 0x7f);

    embx::encoder::Engine encoder(*plan);
    embx::value::Value::Object value;
    value["name"] = embx::value::Value::Bytes{'H','i'};
    value["tail"] = std::uint64_t(0x7f);
    const auto encoded = encoder.encode("Record", value);
    REQUIRE(encoded);
    REQUIRE(encoded.data == wire);
}

TEST_CASE("terminated sequence supports multi-byte terminators") {
    std::string err;
    auto plan = makePlan(R"(struct Line {
  text: bytes until 0x0D 0x0A max 16;
  tail: u8;
})", err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    embx::decoder::Engine decoder(*plan);
    const auto decoded = decoder.decode("Line", std::vector<std::uint8_t>{'H','i',0x0D,0x0A,0x42});
    REQUIRE(decoded);
    REQUIRE(std::get<embx::value::Value::Bytes>(decoded.value.at("text").data) == embx::value::Value::Bytes{'H','i'});
    REQUIRE(std::get<std::uint64_t>(decoded.value.at("tail").data) == 0x42);

    const auto partial = decoder.decode("Line", std::vector<std::uint8_t>{'A',0x0D,'B',0x0D,0x0A,0x42});
    REQUIRE(partial);
    REQUIRE(std::get<embx::value::Value::Bytes>(partial.value.at("text").data) == embx::value::Value::Bytes{'A',0x0D,'B'});
    REQUIRE(std::get<std::uint64_t>(partial.value.at("tail").data) == 0x42);
}

TEST_CASE("terminated sequence failure does not advance the cursor") {
    std::string err;
    auto plan = makePlan(R"(struct Record {
  name: bytes until 0x00 max 3;
  tail: u8;
})", err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    embx::decoder::Engine decoder(*plan);
    const auto missing = decoder.decode("Record", std::vector<std::uint8_t>{'a','b','c','d'});
    REQUIRE_FALSE(missing);
    REQUIRE(missing.consumed == 0);

    const auto tooLong = decoder.decode("Record", std::vector<std::uint8_t>{'a','b','c','d',0x00,0x7f});
    REQUIRE_FALSE(tooLong);
    REQUIRE(tooLong.consumed == 0);

    embx::encoder::Engine encoder(*plan);
    embx::value::Value::Object bad;
    bad["name"] = embx::value::Value::Bytes{'a',0x00,'b'};
    bad["tail"] = std::uint64_t(1);
    const auto encoded = encoder.encode("Record", bad);
    REQUIRE_FALSE(encoded);
}

TEST_CASE("terminated sequence is visible in reflection") {
    std::string err;
    auto plan = makePlan(R"(struct Record {
  name: bytes until 0x00 max 8;
})", err);
    REQUIRE(plan);
    auto reflected = embx::reflection::inspectStruct(*plan, "Record");
    REQUIRE(reflected);
    REQUIRE(reflected->members.size() == 1);
    const auto& member = reflected->members[0];
    REQUIRE(member.terminated);
    REQUIRE(member.terminator == std::vector<std::uint8_t>{0x00});
    REQUIRE(member.maxPayload == 8);
    REQUIRE(member.layoutClass == embx::reflection::LayoutClass::Bounded);
    REQUIRE(member.minSize == 1);
    REQUIRE(member.maxSize == 9);
}


TEST_CASE("terminated sequence accepts empty payload, non-NUL terminators, and terminal at input end") {
    std::string err;
    auto plan = makePlan(R"(struct Record {
  payload: bytes until 0x7E max 4;
})", err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    embx::decoder::Engine decoder(*plan);
    const auto empty = decoder.decode("Record", std::vector<std::uint8_t>{0x7E});
    REQUIRE(empty);
    REQUIRE(std::get<embx::value::Value::Bytes>(empty.value.at("payload").data).empty());
    REQUIRE(empty.consumed == 1);

    const auto final = decoder.decode("Record", std::vector<std::uint8_t>{0x41,0x42,0x7E});
    REQUIRE(final);
    REQUIRE(std::get<embx::value::Value::Bytes>(final.value.at("payload").data) == embx::value::Value::Bytes{0x41,0x42});
    REQUIRE(final.consumed == 3);
}

TEST_CASE("terminated sequence permits maximum payload exactly at the boundary") {
    std::string err;
    auto plan = makePlan(R"(struct Record {
  payload: bytes until 0x00 max 3;
  tail: u8;
})", err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    embx::decoder::Engine decoder(*plan);
    const auto exact = decoder.decode("Record", std::vector<std::uint8_t>{'a','b','c',0x00,0x7F});
    REQUIRE(exact);
    REQUIRE(std::get<embx::value::Value::Bytes>(exact.value.at("payload").data) == embx::value::Value::Bytes{'a','b','c'});
    REQUIRE(std::get<std::uint64_t>(exact.value.at("tail").data) == 0x7F);
    REQUIRE(exact.consumed == 5);
}

TEST_CASE("terminated sequence cannot search outside a bounded containing block") {
    std::string err;
    auto plan = makePlan(R"(struct Container {
  block body[4] {
    payload: bytes until 0x00 max 8;
  }
})", err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    embx::decoder::Engine decoder(*plan);
    const auto result = decoder.decode("Container", std::vector<std::uint8_t>{'a','b','c','d',0x00});
    REQUIRE_FALSE(result);
    REQUIRE(result.consumed == 0);
}

TEST_CASE("nested terminated sequence terminates only between elements") {
    std::string err;
    auto plan = makePlan(R"(struct Entry {
  value: bytes until 0x00 max 8;
}
struct EntryList {
  entries: Entry[*] until 0xFF 0xFF max 32;
  tail: u8;
})", err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    embx::decoder::Engine decoder(*plan);
    const std::vector<std::uint8_t> wire{'A',0x00,'B','C',0x00,0xFF,0xFF,0x7F};
    const auto decoded = decoder.decode("EntryList", wire);
    REQUIRE(decoded);
    REQUIRE(decoded.consumed == wire.size());
    const auto& entries = std::get<embx::value::Value::Array>(decoded.value.at("entries").data);
    REQUIRE(entries.size() == 2);
    REQUIRE(std::get<embx::value::Value::Bytes>(entries[0].at("value").data) == embx::value::Value::Bytes{'A'});
    REQUIRE(std::get<embx::value::Value::Bytes>(entries[1].at("value").data) == embx::value::Value::Bytes{'B','C'});
    REQUIRE(std::get<std::uint64_t>(decoded.value.at("tail").data) == 0x7F);

    embx::value::Value::Object first;
    first["value"] = embx::value::Value::Bytes{'A'};
    embx::value::Value::Object second;
    second["value"] = embx::value::Value::Bytes{'B','C'};
    embx::value::Value::Array values;
    values.emplace_back(std::move(first));
    values.emplace_back(std::move(second));
    embx::value::Value::Object value;
    value["entries"] = std::move(values);
    value["tail"] = std::uint64_t(0x7F);

    embx::encoder::Engine encoder(*plan);
    const auto encoded = encoder.encode("EntryList", value);
    REQUIRE(encoded);
    REQUIRE(encoded.data == wire);

    const auto missingOuter = decoder.decode("EntryList", std::vector<std::uint8_t>{'A',0x00});
    REQUIRE_FALSE(missingOuter);
    REQUIRE(missingOuter.consumed == 0);
}
