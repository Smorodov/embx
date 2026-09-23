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

TEST_CASE("Lesson 4 fixed arrays are statically sized") {
    std::string error;
    auto compilation = embx::compiler::compileFile("../course/04_arrays_dynamic_dimensions/arrays.embx", error);
    REQUIRE(compilation);
    REQUIRE(error.empty());
    REQUIRE(compilation->plan);
    REQUIRE(compilation->plan->structs.size() == 2);

    const auto& fixed = compilation->plan->structs[0];
    REQUIRE(fixed.name == "StaticArray");
    REQUIRE(fixed.staticSize.has_value());
    REQUIRE(*fixed.staticSize == 4);
    REQUIRE(fixed.minSizeInBytes == 4);
    REQUIRE(fixed.maxSizeInBytes.has_value());
    REQUIRE(*fixed.maxSizeInBytes == 4);

    Object root{
        {"values", Value{Array{Value{uint64_t{1}}, Value{uint64_t{2}}, Value{uint64_t{3}}, Value{uint64_t{4}}}}}
    };

    embx::encoder::Engine encoder(*compilation->plan);
    const auto encoded = encoder.encode("StaticArray", Value{root});
    REQUIRE(encoded.success);
    REQUIRE(encoded.data == std::vector<uint8_t>{1, 2, 3, 4});

    embx::decoder::Options options;
    options.requireFullInput = true;
    embx::decoder::Engine decoder(*compilation->plan, options);
    const auto decoded = decoder.decode("StaticArray", encoded.data);
    REQUIRE(decoded.success);
    REQUIRE(decoded.consumed == 4);
    const auto& values = std::get<Array>(field(decoded.value, "values").data);
    REQUIRE(values.size() == 4);
    REQUIRE(number(values[0]) == 1);
    REQUIRE(number(values[3]) == 4);
}

TEST_CASE("Lesson 4 dynamic arrays use an earlier runtime count") {
    std::string error;
    auto compilation = embx::compiler::compileFile("../course/04_arrays_dynamic_dimensions/arrays.embx", error);
    REQUIRE(compilation);
    REQUIRE(error.empty());
    REQUIRE(compilation->plan);

    const auto& dynamic = compilation->plan->structs[1];
    REQUIRE(dynamic.name == "DynamicArray");
    REQUIRE_FALSE(dynamic.staticSize.has_value());
    REQUIRE(dynamic.minSizeInBytes == 1);
    REQUIRE_FALSE(dynamic.maxSizeInBytes.has_value());

    Object root{
        {"count", Value{uint64_t{3}}},
        {"values", Value{Array{
            Value{uint64_t{0x1122}},
            Value{uint64_t{0x3344}},
            Value{uint64_t{0x5566}}
        }}}
    };

    embx::encoder::Engine encoder(*compilation->plan);
    const auto encoded = encoder.encode("DynamicArray", Value{root});
    REQUIRE(encoded.success);
    REQUIRE(encoded.data == std::vector<uint8_t>{0x03, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66});

    embx::decoder::Options options;
    options.requireFullInput = true;
    embx::decoder::Engine decoder(*compilation->plan, options);
    const auto decoded = decoder.decode("DynamicArray", encoded.data);
    REQUIRE(decoded.success);
    REQUIRE(decoded.consumed == 7);

    REQUIRE(number(field(decoded.value, "count")) == 3);
    const auto& values = std::get<Array>(field(decoded.value, "values").data);
    REQUIRE(values.size() == 3);
    REQUIRE(number(values[0]) == 0x1122);
    REQUIRE(number(values[1]) == 0x3344);
    REQUIRE(number(values[2]) == 0x5566);
}
