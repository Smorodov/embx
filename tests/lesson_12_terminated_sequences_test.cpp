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
    const auto path = std::filesystem::path("../course/12_terminated_sequences/terminated_sequences.embx");
    auto compilation = embx::compiler::compileFile(path.string(), error);
    if (!compilation) return {};
    return std::shared_ptr<embx::plan::Module>(std::move(compilation->plan));
}

std::vector<std::uint8_t> fixture() {
    std::ifstream in("../course/12_terminated_sequences/terminated_sequences.bin", std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}
}

TEST_CASE("lesson 12 source compiles into an executable plan") {
    std::string error;
    auto plan = compileLesson(error);
    REQUIRE(plan);
    REQUIRE(error.empty());
}

TEST_CASE("nested terminated sequence decodes only at element boundaries") {
    std::string error;
    auto plan = compileLesson(error);
    REQUIRE(plan);

    const auto wire = fixture();
    embx::decoder::Engine decoder(*plan);
    const auto decoded = decoder.decode("EntryList", wire);
    REQUIRE(decoded);
    REQUIRE(decoded.consumed == wire.size());

    const auto& entries = std::get<embx::value::Value::Array>(decoded.value.at("entries").data);
    REQUIRE(entries.size() == 2);
    REQUIRE(std::get<embx::value::Value::Bytes>(entries[0].at("value").data) == embx::value::Value::Bytes{0x41});
    REQUIRE(std::get<embx::value::Value::Bytes>(entries[1].at("value").data) == embx::value::Value::Bytes{0x42, 0x43});
    REQUIRE(std::get<std::uint64_t>(decoded.value.at("tail").data) == 0x7F);
}

TEST_CASE("lesson 12 round trips the real fixture and requires the outer terminator") {
    std::string error;
    auto plan = compileLesson(error);
    REQUIRE(plan);

    const auto wire = fixture();
    embx::decoder::Engine decoder(*plan);
    const auto decoded = decoder.decode("EntryList", wire);
    REQUIRE(decoded);

    embx::encoder::Engine encoder(*plan);
    const auto encoded = encoder.encode("EntryList", decoded.value);
    REQUIRE(encoded);
    REQUIRE(encoded.data == wire);

    auto truncated = wire;
    truncated.erase(truncated.end() - 3, truncated.end() - 1);
    const auto missing = decoder.decode("EntryList", truncated);
    REQUIRE_FALSE(missing);
    REQUIRE(missing.consumed == 0);
}
