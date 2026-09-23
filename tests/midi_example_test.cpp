#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "compiler/Compiler.h"
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#ifndef EMBX_SOURCE_DIR
#define EMBX_SOURCE_DIR "."
#endif

using Value = embx::value::Value;

static const Value& field(const Value& v, const char* name) {
    return std::get<Value::Object>(v.data).at(name);
}

static std::uint64_t u(const Value& v) {
    REQUIRE(v.kind() == Value::Kind::Unsigned);
    return std::get<std::uint64_t>(v.data);
}

TEST_CASE("MIDI structural example decodes the real fixture", "[examples][midi]") {
    const std::string schema = std::string(EMBX_SOURCE_DIR) + "/examples/midi.embx";
    const std::string fixture = std::string(EMBX_SOURCE_DIR) + "/examples/midi_demo.mid";

    std::string error;
    auto compilation = embx::compiler::compileFile(schema, error);
    REQUIRE(compilation);
    REQUIRE(error.empty());

    std::ifstream in(fixture, std::ios::binary);
    REQUIRE(in);
    std::vector<std::uint8_t> bytes;
    for (std::istreambuf_iterator<char> it(in), end; it != end; ++it)
        bytes.push_back(static_cast<std::uint8_t>(static_cast<unsigned char>(*it)));
    REQUIRE(bytes.size() == 69);

    embx::decoder::Options options;
    options.requireFullInput = true;
    embx::decoder::Engine decoder(*compilation->plan, options);
    const auto result = decoder.decode("examples::midi::MidiFile", bytes);

    REQUIRE(result.success);
    REQUIRE(result.consumed == bytes.size());
    REQUIRE(field(result.value, "magic").kind() == Value::Kind::Bytes);
    REQUIRE(u(field(result.value, "header_length")) == 6);
    REQUIRE(u(field(result.value, "format")) == 0);
    REQUIRE(u(field(result.value, "track_count")) == 1);
    REQUIRE(u(field(result.value, "division")) == 480);

    const auto& tracks = std::get<Value::Array>(field(result.value, "tracks").data);
    REQUIRE(tracks.size() == 1);
    const auto& trackObject = std::get<Value::Object>(tracks[0].data);
    const auto lengthIt = trackObject.find("length");
    REQUIRE(lengthIt != trackObject.end());
    REQUIRE(u(lengthIt->second) == 47);

    const auto rawIt = trackObject.find("raw");
    REQUIRE(rawIt != trackObject.end());
    const auto& raw = std::get<Value::Bytes>(rawIt->second.data);
    REQUIRE(raw.size() == 47);
    REQUIRE(raw[0] == 0x00);
    REQUIRE(raw[1] == 0xff);
    REQUIRE(raw[2] == 0x51);
    REQUIRE(raw[3] == 0x03);
    REQUIRE(raw[4] == 0x07);
    REQUIRE(raw[5] == 0xa1);
    REQUIRE(raw[6] == 0x20);
    REQUIRE(raw[7] == 0x00);
    REQUIRE(raw[8] == 0x90);
    REQUIRE(raw[9] == 0x3c);
    REQUIRE(raw[10] == 0x60);
    REQUIRE(raw[11] == 0x83);
    REQUIRE(raw[12] == 0x60);
    REQUIRE(raw[13] == 0x80);
    REQUIRE(raw[14] == 0x3c);
    REQUIRE(raw[15] == 0x40);
    REQUIRE(raw.back() == 0x00);

    embx::encoder::Engine encoder(*compilation->plan);
    const auto encoded = encoder.encode("examples::midi::MidiFile", result.value);
    REQUIRE(encoded.success);
    REQUIRE(encoded.data == bytes);
}
