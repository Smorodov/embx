#include <catch2/catch_test_macros.hpp>
#include "compiler/Compiler.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "value/Value.h"
#include <cstdint>
#include <vector>

using Value = embx::value::Value;
using Object = Value::Object;
using Array = Value::Array;

static const Value& field(const Value& v, const char* name) {
    return std::get<Object>(v.data).at(name);
}

static uint64_t number(const Value& v) {
    return std::get<uint64_t>(v.data);
}

static const Value& nestedField(const Value& v, const char* name) {
    return field(field(v, "header"), name);
}

TEST_CASE("Lesson 5 nested structures compose exact layouts") {
    std::string error;
    auto compilation = embx::compiler::compileFile("../course/05_nested_structures/nested.embx", error);
    REQUIRE(compilation);
    REQUIRE(error.empty());
    REQUIRE(compilation->plan);
    REQUIRE(compilation->plan->structs.size() == 2);

    const auto& header = compilation->plan->structs[0];
    REQUIRE(header.name == "Header");
    REQUIRE(header.staticSize.has_value());
    REQUIRE(*header.staticSize == 2);
    REQUIRE(header.minSizeInBytes == 2);
    REQUIRE(header.maxSizeInBytes.has_value());
    REQUIRE(*header.maxSizeInBytes == 2);

    const auto& packet = compilation->plan->structs[1];
    REQUIRE(packet.name == "Packet");
    REQUIRE(packet.staticSize.has_value());
    REQUIRE(*packet.staticSize == 7);
    REQUIRE(packet.minSizeInBytes == 7);
    REQUIRE(packet.maxSizeInBytes.has_value());
    REQUIRE(*packet.maxSizeInBytes == 7);

    Object headerValue{
        {"version", Value{uint64_t{1}}},
        {"flags", Value{uint64_t{2}}}
    };
    Object root{
        {"header", Value{headerValue}},
        {"sequence", Value{uint64_t{0x1234}}},
        {"payload", Value{Array{
            Value{uint64_t{0xAA}},
            Value{uint64_t{0xBB}},
            Value{uint64_t{0xCC}}
        }}}
    };

    embx::encoder::Engine encoder(*compilation->plan);
    const auto encoded = encoder.encode("Packet", Value{root});
    REQUIRE(encoded.success);
    REQUIRE(encoded.data == std::vector<uint8_t>{0x01, 0x02, 0x12, 0x34, 0xAA, 0xBB, 0xCC});

    embx::decoder::Options options;
    options.requireFullInput = true;
    embx::decoder::Engine decoder(*compilation->plan, options);
    const auto decoded = decoder.decode("Packet", encoded.data);
    REQUIRE(decoded.success);
    REQUIRE(decoded.consumed == 7);

    REQUIRE(number(nestedField(decoded.value, "version")) == 1);
    REQUIRE(number(nestedField(decoded.value, "flags")) == 2);
    REQUIRE(number(field(decoded.value, "sequence")) == 0x1234);

    const auto& payload = std::get<Array>(field(decoded.value, "payload").data);
    REQUIRE(payload.size() == 3);
    REQUIRE(number(payload[0]) == 0xAA);
    REQUIRE(number(payload[1]) == 0xBB);
    REQUIRE(number(payload[2]) == 0xCC);
}

TEST_CASE("Lesson 5 nested structures preserve object boundaries") {
    std::string error;
    auto compilation = embx::compiler::compileFile("../course/05_nested_structures/nested.embx", error);
    REQUIRE(compilation);
    REQUIRE(error.empty());
    REQUIRE(compilation->plan);

    Object root{
        {"header", Value{Object{
            {"version", Value{uint64_t{9}}},
            {"flags", Value{uint64_t{7}}}
        }}},
        {"sequence", Value{uint64_t{0x0102}}},
        {"payload", Value{Array{
            Value{uint64_t{0x10}}, Value{uint64_t{0x20}}, Value{uint64_t{0x30}}
        }}}
    };

    embx::encoder::Engine encoder(*compilation->plan);
    const auto encoded = encoder.encode("Packet", Value{root});
    REQUIRE(encoded.success);

    embx::decoder::Options options;
    options.requireFullInput = true;
    embx::decoder::Engine decoder(*compilation->plan, options);
    const auto decoded = decoder.decode("Packet", encoded.data);
    REQUIRE(decoded.success);

    const auto& object = std::get<Object>(decoded.value.data);
    REQUIRE(object.count("header") == 1);
    REQUIRE(object.count("sequence") == 1);
    REQUIRE(object.count("payload") == 1);

    const auto& nested = std::get<Object>(object.at("header").data);
    REQUIRE(nested.size() == 2);
    REQUIRE(number(nested.at("version")) == 9);
    REQUIRE(number(nested.at("flags")) == 7);
}
