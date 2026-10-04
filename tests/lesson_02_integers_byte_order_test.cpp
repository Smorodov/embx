#include <catch2/catch_test_macros.hpp>
#include "compiler/Compiler.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "value/Value.h"
#include <cstdint>
#include <vector>

using Value = embx::value::Value;
using Object = Value::Object;

static const Value& field(const Value& v, const char* name) {
    return std::get<Object>(v.data).at(name);
}

TEST_CASE("Lesson 2 integers and byte order are executable end to end") {
    std::string error;
    auto compilation = embx::compiler::compileFile("../course/02_integers_byte_order/message.embx", error);
    REQUIRE(compilation);
    REQUIRE(error.empty());
    REQUIRE(compilation->plan);
    REQUIRE(compilation->plan->structs.size() == 1);

    const auto& numbers = compilation->plan->structs.front();
    REQUIRE(numbers.name == "Numbers");
    REQUIRE(numbers.staticSize.has_value());
    REQUIRE(*numbers.staticSize == 7);
    REQUIRE(numbers.minSizeInBytes == 7);
    REQUIRE(numbers.maxSizeInBytes.has_value());
    REQUIRE(*numbers.maxSizeInBytes == 7);

    Object root;
    root["bigValue"] = Value{uint64_t{0x1234}};
    root["littleValue"] = Value{uint64_t{0x5678}};
    root["signedLittle"] = Value{int64_t{-2}};
    root["byteValue"] = Value{uint64_t{0xA5}};

    embx::encoder::Engine encoder(*compilation->plan);
    const auto encoded = encoder.encode("Numbers", Value{root});
    REQUIRE(encoded.success);

    const std::vector<uint8_t> expected{0x12, 0x34, 0x78, 0x56, 0xFE, 0xFF, 0xA5};
    REQUIRE(encoded.data == expected);

    embx::decoder::Options options;
    options.requireFullInput = true;
    embx::decoder::Engine decoder(*compilation->plan, options);
    const auto decoded = decoder.decode("Numbers", encoded.data);
    REQUIRE(decoded.success);
    REQUIRE(decoded.consumed == expected.size());

    REQUIRE(std::get<uint64_t>(field(decoded.value, "bigValue").data) == 0x1234);
    REQUIRE(std::get<uint64_t>(field(decoded.value, "littleValue").data) == 0x5678);
    REQUIRE(std::get<int64_t>(field(decoded.value, "signedLittle").data) == -2);
    REQUIRE(std::get<uint64_t>(field(decoded.value, "byteValue").data) == 0xA5);
}
