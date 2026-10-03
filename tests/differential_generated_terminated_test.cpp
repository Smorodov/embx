#include <catch2/catch_test_macros.hpp>
#include "compiler/Compiler.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "value/Value.h"
#include "generated.hpp"
#include <cstdint>
#include <vector>

using Value = embx::value::Value;
using Object = Value::Object;
using Array = Value::Array;

static const Value& field(const Value& v, const char* name) {
    return std::get<Object>(v.data).at(name);
}

static std::unique_ptr<embx::compiler::Compilation> compileTerminated(std::string& error) {
    return embx::compiler::compileFile("../course/12_terminated_sequences/terminated_sequences.embx", error);
}

TEST_CASE("generated C++ matches Reference Runtime for nested terminated sequences") {
    std::string error;
    auto c = compileTerminated(error);
    REQUIRE(c);
    REQUIRE(error.empty());

    Object first{{"value", Value::Bytes{'A'}}};
    Object second{{"value", Value::Bytes{'B', 'C'}}};
    Array entries{Value{first}, Value{second}};
    Object root{
        {"entries", Value{entries}},
        {"tail", uint64_t{0x7F}}
    };

    auto ref = embx::encoder::Engine(*c->plan).encode("EntryList", Value{root});
    REQUIRE(ref);
    const std::vector<std::uint8_t> expected{'A', 0x00, 'B', 'C', 0x00, 0xFF, 0xFF, 0x7F};
    REQUIRE(ref.data == expected);

    EntryList generatedValue{};
    Entry firstGenerated{};
    firstGenerated.value = {'A'};
    Entry secondGenerated{};
    secondGenerated.value = {'B', 'C'};
    generatedValue.entries = {firstGenerated, secondGenerated};
    generatedValue.tail = 0x7F;

    std::vector<std::uint8_t> generatedBytes;
    REQUIRE(encode__EntryList(generatedValue, generatedBytes));
    REQUIRE(generatedBytes == ref.data);

    embx::decoder::Options options;
    options.requireFullInput = true;
    auto decoded = embx::decoder::Engine(*c->plan, options).decode("EntryList", ref.data);
    REQUIRE(decoded);
    REQUIRE(decoded.consumed == ref.data.size());

    EntryList generatedDecoded{};
    std::size_t consumed = 0;
    REQUIRE(decode__EntryList(ref.data, generatedDecoded, consumed));
    REQUIRE(consumed == ref.data.size());
    REQUIRE(generatedDecoded.entries.size() == 2);
    REQUIRE(generatedDecoded.entries[0].value == std::vector<std::uint8_t>{'A'});
    REQUIRE(generatedDecoded.entries[1].value == std::vector<std::uint8_t>{'B', 'C'});
    REQUIRE(generatedDecoded.tail == std::get<std::uint64_t>(field(decoded.value, "tail").data));
}

TEST_CASE("generated C++ preserves transactional failure for missing outer terminator") {
    std::string error;
    auto c = compileTerminated(error);
    REQUIRE(c);
    REQUIRE(error.empty());

    const std::vector<std::uint8_t> invalid{'A', 0x00, 'B', 'C', 0x00};
    auto ref = embx::decoder::Engine(*c->plan).decode("EntryList", invalid);
    REQUIRE_FALSE(ref);
    REQUIRE(ref.consumed == 0);

    EntryList generatedDecoded{};
    std::size_t consumed = 123;
    std::string generatedError;
    REQUIRE_FALSE(decode__EntryList(invalid, generatedDecoded, consumed, nullptr, &generatedError));
    REQUIRE(consumed == 0);
}
