#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "compiler/Compiler.h"
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"

namespace {
std::shared_ptr<embx::plan::Module> compileLesson(std::string& error) {
    const auto path = std::filesystem::path("../course/14_tlv/tlv.embx");
    auto compilation = embx::compiler::compileFile(path.string(), error);
    if (!compilation) return {};
    return std::shared_ptr<embx::plan::Module>(std::move(compilation->plan));
}

std::vector<std::uint8_t> readFixture(const char* name) {
    std::ifstream in(std::string("../course/14_tlv/") + name, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}
}

TEST_CASE("lesson 14 TLV source compiles into an executable plan") {
    std::string error;
    auto plan = compileLesson(error);
    REQUIRE(plan);
    REQUIRE(error.empty());
}

TEST_CASE("TLV fixture decodes through length-dependent payloads") {
    std::string error;
    auto plan = compileLesson(error);
    REQUIRE(plan);

    const auto wire = readFixture("tlv.bin");
    embx::decoder::Engine decoder(*plan);
    const auto decoded = decoder.decode("Message", wire);
    REQUIRE(decoded);
    REQUIRE(decoded.consumed == wire.size());

    const auto& records = std::get<embx::value::Value::Array>(decoded.value.at("records").data);
    REQUIRE(records.size() == 3);
    REQUIRE(std::get<std::uint64_t>(records[0].at("kind").data) == 1);
    REQUIRE(std::get<std::uint64_t>(records[0].at("length").data) == 3);
    REQUIRE(std::get<embx::value::Value::Bytes>(records[0].at("data").data) == embx::value::Value::Bytes{0x41, 0x42, 0x43});
    REQUIRE(std::get<std::uint64_t>(records[1].at("kind").data) == 2);
    REQUIRE(std::get<std::uint64_t>(records[1].at("length").data) == 4);
    REQUIRE(std::get<embx::value::Value::Bytes>(records[1].at("data").data) == embx::value::Value::Bytes{0xDE, 0xAD, 0xBE, 0xEF});
    REQUIRE(std::get<std::uint64_t>(records[2].at("kind").data) == 3);
    REQUIRE(std::get<std::uint64_t>(records[2].at("length").data) == 0);
    REQUIRE(std::get<embx::value::Value::Bytes>(records[2].at("data").data).empty());
}

TEST_CASE("TLV round trip preserves the real fixture") {
    std::string error;
    auto plan = compileLesson(error);
    REQUIRE(plan);

    const auto wire = readFixture("tlv.bin");
    embx::decoder::Engine decoder(*plan);
    const auto decoded = decoder.decode("Message", wire);
    REQUIRE(decoded);

    embx::encoder::Engine encoder(*plan);
    const auto encoded = encoder.encode("Message", decoded.value);
    REQUIRE(encoded);
    REQUIRE(encoded.data == wire);
}

TEST_CASE("TLV outer terminator is checked only between records") {
    std::string error;
    auto plan = compileLesson(error);
    REQUIRE(plan);

    const auto wire = readFixture("tlv_inner_terminator.bin");
    embx::decoder::Engine decoder(*plan);
    const auto decoded = decoder.decode("Message", wire);
    REQUIRE(decoded);
    REQUIRE(decoded.consumed == wire.size());
    const auto& records = std::get<embx::value::Value::Array>(decoded.value.at("records").data);
    REQUIRE(records.size() == 2);
    REQUIRE(std::get<embx::value::Value::Bytes>(records[0].at("data").data) == embx::value::Value::Bytes{0x41, 0xFF, 0xFF, 0x42, 0x43});
    REQUIRE(std::get<embx::value::Value::Bytes>(records[1].at("data").data) == embx::value::Value::Bytes{0x7F});
}

TEST_CASE("TLV rejects a length that exceeds the available payload") {
    std::string error;
    auto plan = compileLesson(error);
    REQUIRE(plan);

    const auto wire = readFixture("tlv_invalid_length.bin");
    embx::decoder::Engine decoder(*plan);
    const auto decoded = decoder.decode("Message", wire);
    REQUIRE_FALSE(decoded);
    REQUIRE(decoded.consumed == 0);
}
