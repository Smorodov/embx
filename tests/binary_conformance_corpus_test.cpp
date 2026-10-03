#include <catch2/catch_test_macros.hpp>
#include "compiler/Compiler.h"
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {
struct ValidCase {
    const char* schema;
    const char* type;
    const char* fixture;
};

std::vector<std::uint8_t> readFixture(const char* path) {
    std::ifstream in(path, std::ios::binary);
    REQUIRE(in);
    return {std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
}

void requireRoundTrip(const ValidCase& c) {
    std::string error;
    auto compilation = embx::compiler::compileFile(c.schema, error);
    REQUIRE(compilation);
    REQUIRE(error.empty());
    REQUIRE(compilation->plan);

    const auto wire = readFixture(c.fixture);
    REQUIRE_FALSE(wire.empty());

    embx::decoder::Options options;
    options.requireFullInput = true;
    embx::decoder::Engine decoder(*compilation->plan, options);
    const auto decoded = decoder.decode(c.type, wire);
    REQUIRE(decoded.success);
    REQUIRE(decoded.consumed == wire.size());

    embx::encoder::Engine encoder(*compilation->plan);
    const auto encoded = encoder.encode(c.type, decoded.value);
    REQUIRE(encoded.success);
    REQUIRE(encoded.data == wire);
}
}

TEST_CASE("binary conformance corpus: valid fixtures decode and re-encode identically", "[conformance][binary]") {
    const ValidCase cases[] = {
        {"../course/01_first_format/message.embx", "Message", "../course/01_first_format/message.bin"},
        {"../course/11_ipv4/ipv4.embx", "net::ipv4::Header", "../course/11_ipv4/ipv4_header.bin"},
        {"../course/12_terminated_sequences/terminated_sequences.embx", "EntryList", "../course/12_terminated_sequences/terminated_sequences.bin"},
        {"../course/14_tlv/tlv.embx", "Message", "../course/14_tlv/tlv.bin"},
        {"../examples/midi.embx", "examples::midi::MidiFile", "../examples/midi_demo.mid"}
    };

    for (const auto& c : cases) {
        DYNAMIC_SECTION(c.schema << " :: " << c.fixture) {
            requireRoundTrip(c);
        }
    }
}

TEST_CASE("binary conformance corpus: invalid fixture is rejected", "[conformance][binary]") {
    std::string error;
    auto compilation = embx::compiler::compileFile("../course/14_tlv/tlv.embx", error);
    REQUIRE(compilation);
    REQUIRE(error.empty());

    const auto wire = readFixture("../course/14_tlv/tlv_invalid_length.bin");
    embx::decoder::Options options;
    options.requireFullInput = true;
    embx::decoder::Engine decoder(*compilation->plan, options);
    const auto decoded = decoder.decode("Message", wire);
    REQUIRE_FALSE(decoded.success);
}
