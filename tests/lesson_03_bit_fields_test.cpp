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

TEST_CASE("Lesson 3 bit fields are executable end to end") {
    std::string error;
    auto compilation = embx::compiler::compileFile("../course/03_bit_fields/flags.embx", error);
    REQUIRE(compilation);
    REQUIRE(error.empty());
    REQUIRE(compilation->plan);
    REQUIRE(compilation->plan->structs.size() == 1);

    const auto& flags = compilation->plan->structs.front();
    REQUIRE(flags.name == "Flags");
    REQUIRE(flags.staticSize.has_value());
    REQUIRE(*flags.staticSize == 1);
    REQUIRE(flags.minSizeInBytes == 1);
    REQUIRE(flags.maxSizeInBytes.has_value());
    REQUIRE(*flags.maxSizeInBytes == 1);

    Object root;
    root["version"] = Value{uint64_t{5}};
    root["kind"] = Value{uint64_t{2}};
    root["enabled"] = Value{uint64_t{1}};
    root["code"] = Value{uint64_t{1}};

    embx::encoder::Engine encoder(*compilation->plan);
    const auto encoded = encoder.encode("Flags", Value{root});
    REQUIRE(encoded.success);

    const std::vector<uint8_t> expected{0xB5};
    REQUIRE(encoded.data == expected);

    embx::decoder::Options options;
    options.requireFullInput = true;
    embx::decoder::Engine decoder(*compilation->plan, options);
    const auto decoded = decoder.decode("Flags", encoded.data);
    REQUIRE(decoded.success);
    REQUIRE(decoded.consumed == expected.size());

    REQUIRE(std::get<uint64_t>(field(decoded.value, "version").data) == 5);
    REQUIRE(std::get<uint64_t>(field(decoded.value, "kind").data) == 2);
    REQUIRE(std::get<uint64_t>(field(decoded.value, "enabled").data) == 1);
    REQUIRE(std::get<uint64_t>(field(decoded.value, "code").data) == 1);
}
