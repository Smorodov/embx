#include <catch2/catch_test_macros.hpp>
#include "compiler/Compiler.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "value/Value.h"
#include <cstdint>
#include <vector>

using Value = embx::value::Value;
using Object = Value::Object;

static uint64_t number(const Value& v) {
    return std::get<uint64_t>(v.data);
}

TEST_CASE("Lesson 7 at, next, and alignment compose into one physical layout") {
    std::string error;
    auto compilation = embx::compiler::compileFile("../course/07_offsets_alignment/offsets_alignment.embx", error);
    REQUIRE(compilation);
    REQUIRE(error.empty());
    REQUIRE(compilation->plan);

    embx::encoder::Engine encoder(*compilation->plan);
    Object input{
        {"head", Value{uint64_t{0x07}}},
        {"marker", Value{uint64_t{0xAA}}},
        {"value", Value{uint64_t{0x1122}}},
        {"tail", Value{uint64_t{0x08}}}
    };

    auto encoded = encoder.encode("OffsetAlign", Value{input});
    REQUIRE(encoded.success);
    REQUIRE(encoded.data == std::vector<uint8_t>{0x07, 0x08, 0x00, 0xAA, 0x11, 0x22});

    embx::decoder::Options options;
    options.requireFullInput = false;
    embx::decoder::Engine decoder(*compilation->plan, options);
    auto decoded = decoder.decode("OffsetAlign", encoded.data);
    REQUIRE(decoded.success);
    REQUIRE(decoded.consumed == 2);

    const auto& object = std::get<Object>(decoded.value.data);
    REQUIRE(number(object.at("head")) == 0x07);
    REQUIRE(number(object.at("tail")) == 0x08);
    REQUIRE(number(object.at("marker")) == 0xAA);
    REQUIRE(number(object.at("value")) == 0x1122);
}
