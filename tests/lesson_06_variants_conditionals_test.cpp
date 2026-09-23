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

static const Value& field(const Value& v, const char* name) {
    return std::get<Object>(v.data).at(name);
}

TEST_CASE("Lesson 6 variants select explicit and default cases") {
    std::string error;
    auto compilation = embx::compiler::compileFile("../course/06_variants_conditionals/variants.embx", error);
    REQUIRE(compilation);
    REQUIRE(error.empty());
    REQUIRE(compilation->plan);

    embx::encoder::Engine encoder(*compilation->plan);

    Object explicitCase{
        {"kind", Value{uint64_t{1}}},
        {"body", Value{Object{{"value", Value{uint64_t{0x1234}}}}}}
    };
    auto encodedExplicit = encoder.encode("Packet", Value{explicitCase});
    REQUIRE(encodedExplicit.success);
    REQUIRE(encodedExplicit.data == std::vector<uint8_t>{0x01, 0x12, 0x34});

    embx::decoder::Options options;
    options.requireFullInput = true;
    embx::decoder::Engine decoder(*compilation->plan, options);
    auto decodedExplicit = decoder.decode("Packet", encodedExplicit.data);
    REQUIRE(decodedExplicit.success);
    REQUIRE(decodedExplicit.consumed == 3);
    REQUIRE(number(field(decodedExplicit.value, "kind")) == 1);
    REQUIRE(number(field(field(decodedExplicit.value, "body"), "value")) == 0x1234);

    Object defaultCase{
        {"kind", Value{uint64_t{9}}},
        {"body", Value{Object{{"value", Value{uint64_t{7}}}}}}
    };
    auto encodedDefault = encoder.encode("Packet", Value{defaultCase});
    REQUIRE(encodedDefault.success);
    REQUIRE(encodedDefault.data == std::vector<uint8_t>{0x09, 0x07});

    auto decodedDefault = decoder.decode("Packet", encodedDefault.data);
    REQUIRE(decodedDefault.success);
    REQUIRE(decodedDefault.consumed == 2);
    REQUIRE(number(field(field(decodedDefault.value, "body"), "value")) == 7);
}

TEST_CASE("Lesson 6 conditionals select only the active branch") {
    std::string error;
    auto compilation = embx::compiler::compileFile("../course/06_variants_conditionals/variants.embx", error);
    REQUIRE(compilation);
    REQUIRE(error.empty());
    REQUIRE(compilation->plan);

    embx::encoder::Engine encoder(*compilation->plan);

    Object thenObject{
        {"flags", Value{uint64_t{1}}},
        {"value", Value{uint64_t{0x11223344}}}
    };
    auto encodedThen = encoder.encode("ConditionalPacket", Value{thenObject});
    REQUIRE(encodedThen.success);
    REQUIRE(encodedThen.data == std::vector<uint8_t>{0x01, 0x11, 0x22, 0x33, 0x44});

    embx::decoder::Options options;
    options.requireFullInput = true;
    embx::decoder::Engine decoder(*compilation->plan, options);
    auto decodedThen = decoder.decode("ConditionalPacket", encodedThen.data);
    REQUIRE(decodedThen.success);
    REQUIRE(decodedThen.consumed == 5);
    REQUIRE(number(field(decodedThen.value, "value")) == 0x11223344);
    REQUIRE(decodedThen.value.get("small") == nullptr);

    Object elseObject{
        {"flags", Value{uint64_t{0}}},
        {"small", Value{uint64_t{0x5566}}}
    };
    auto encodedElse = encoder.encode("ConditionalPacket", Value{elseObject});
    REQUIRE(encodedElse.success);
    REQUIRE(encodedElse.data == std::vector<uint8_t>{0x00, 0x55, 0x66});

    auto decodedElse = decoder.decode("ConditionalPacket", encodedElse.data);
    REQUIRE(decodedElse.success);
    REQUIRE(decodedElse.consumed == 3);
    REQUIRE(number(field(decodedElse.value, "small")) == 0x5566);
    REQUIRE(decodedElse.value.get("value") == nullptr);
}
