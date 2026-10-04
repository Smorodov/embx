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

static std::uint64_t number(const Value& v) {
    return std::get<std::uint64_t>(v.data);
}

static std::unique_ptr<embx::compiler::Compilation> compileLayout(std::string& error) {
    return embx::compiler::compileFile("../tests/fixtures/differential_layout.embx", error);
}

TEST_CASE("generated C++ matches Reference Runtime for dynamic dimensions and explicit layout") {
    std::string error;
    auto c = compileLayout(error);
    REQUIRE(c);
    REQUIRE(error.empty());

    Object root{
        {"count", uint64_t{3}},
        {"values", Value{Array{Value{uint64_t{0x11}}, Value{uint64_t{0x22}}, Value{uint64_t{0x33}}}}},
        {"offset", uint64_t{8}},
        {"marker", uint64_t{0xA5}},
        {"value", uint64_t{0x1234}},
        {"tail", uint64_t{0x7E}}
    };

    auto ref = embx::encoder::Engine(*c->plan).encode("differential_layout::LayoutPacket", Value{root});
    REQUIRE(ref);
    REQUIRE(ref.data == std::vector<std::uint8_t>{
        0x03, 0x11, 0x22, 0x33, 0x08, 0x7E,
        0x00, 0x00, 0xA5, 0x00, 0x00, 0x00,
        0x12, 0x34
    });

    differential_layout::LayoutPacket generatedValue{};
    generatedValue.count = 3;
    generatedValue.values = {0x11, 0x22, 0x33};
    generatedValue.offset = 8;
    generatedValue.marker = 0xA5;
    generatedValue.value = 0x1234;
    generatedValue.tail = 0x7E;

    std::vector<std::uint8_t> generatedBytes;
    REQUIRE(differential_layout::encode__differential_layout__LayoutPacket(generatedValue, generatedBytes));
    REQUIRE(generatedBytes == ref.data);

    embx::decoder::Options options;
    options.requireFullInput = false;
    auto decoded = embx::decoder::Engine(*c->plan, options).decode(
        "differential_layout::LayoutPacket", ref.data);
    REQUIRE(decoded);
    REQUIRE(decoded.consumed == 6);

    differential_layout::LayoutPacket generatedDecoded{};
    std::size_t consumed = 0;
    REQUIRE(differential_layout::decode__differential_layout__LayoutPacket(
        ref.data, generatedDecoded, consumed));
    REQUIRE(consumed == 6);
    REQUIRE(generatedDecoded.count == number(field(decoded.value, "count")));
    REQUIRE(generatedDecoded.values.size() == 3);
    REQUIRE(generatedDecoded.offset == 8);
    REQUIRE(generatedDecoded.values[0] == 0x11);
    REQUIRE(generatedDecoded.values[2] == 0x33);
    REQUIRE(generatedDecoded.marker == number(field(decoded.value, "marker")));
    REQUIRE(generatedDecoded.value == number(field(decoded.value, "value")));
    REQUIRE(generatedDecoded.tail == number(field(decoded.value, "tail")));
}

TEST_CASE("generated C++ preserves zero-length dynamic array layout") {
    std::string error;
    auto c = compileLayout(error);
    REQUIRE(c);
    REQUIRE(error.empty());

    Object root{
        {"count", uint64_t{0}},
        {"values", Value{Array{}}},
        {"offset", uint64_t{5}},
        {"marker", uint64_t{0xA5}},
        {"value", uint64_t{0x1234}},
        {"tail", uint64_t{0x7E}}
    };

    auto ref = embx::encoder::Engine(*c->plan).encode("differential_layout::LayoutPacket", Value{root});
    REQUIRE(ref);
    REQUIRE(ref.data == std::vector<std::uint8_t>{0x00, 0x05, 0x7E, 0x00, 0x00, 0xA5, 0x00, 0x00, 0x12, 0x34});

    differential_layout::LayoutPacket generatedValue{};
    generatedValue.count = 0;
    generatedValue.values = {};
    generatedValue.offset = 5;
    generatedValue.marker = 0xA5;
    generatedValue.value = 0x1234;
    generatedValue.tail = 0x7E;

    std::vector<std::uint8_t> generatedBytes;
    REQUIRE(differential_layout::encode__differential_layout__LayoutPacket(generatedValue, generatedBytes));
    REQUIRE(generatedBytes == ref.data);

    differential_layout::LayoutPacket generatedDecoded{};
    std::size_t consumed = 0;
    REQUIRE(differential_layout::decode__differential_layout__LayoutPacket(
        ref.data, generatedDecoded, consumed));
    REQUIRE(consumed == 3);
    REQUIRE(generatedDecoded.count == 0);
    REQUIRE(generatedDecoded.values.empty());
    REQUIRE(generatedDecoded.offset == 5);
    REQUIRE(generatedDecoded.marker == 0xA5);
    REQUIRE(generatedDecoded.value == 0x1234);
    REQUIRE(generatedDecoded.tail == 0x7E);
}
