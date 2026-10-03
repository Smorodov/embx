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

static uint64_t integer(const Value& v) {
    return std::get<uint64_t>(v.data);
}

static std::vector<uint8_t> bytes(const Value& v) {
    return std::get<Value::Bytes>(v.data);
}

TEST_CASE("Lesson 1 first binary format is executable end to end") {
    std::string error;
    auto compilation = embx::compiler::compileFile("../course/01_first_format/message.embx", error);
    REQUIRE(compilation);
    REQUIRE(error.empty());
    REQUIRE(compilation->plan);
    REQUIRE(compilation->plan->structs.size() == 1);

    const auto& message = compilation->plan->structs.front();
    REQUIRE(message.name == "Message");
    REQUIRE(message.staticSize.has_value());
    REQUIRE(*message.staticSize == 7);
    REQUIRE(message.minSizeInBytes == 7);
    REQUIRE(message.maxSizeInBytes.has_value());
    REQUIRE(*message.maxSizeInBytes == 7);

    Object root;
    root["magic"] = Value{uint64_t{0xCAFE}};
    root["version"] = Value{uint64_t{1}};
    root["payload"] = Value{Value::Bytes{0xDE, 0xAD, 0xBE, 0xEF}};

    embx::encoder::Engine encoder(*compilation->plan);
    const auto encoded = encoder.encode("Message", Value{root});
    REQUIRE(encoded.success);

    const std::vector<uint8_t> expected{0xCA, 0xFE, 0x01, 0xDE, 0xAD, 0xBE, 0xEF};
    REQUIRE(encoded.data == expected);

    embx::decoder::Options options;
    options.requireFullInput = true;
    embx::decoder::Engine decoder(*compilation->plan, options);
    const auto decoded = decoder.decode("Message", encoded.data);
    REQUIRE(decoded.success);
    REQUIRE(decoded.consumed == expected.size());
    REQUIRE(integer(field(decoded.value, "magic")) == 0xCAFE);
    REQUIRE(integer(field(decoded.value, "version")) == 1);
    REQUIRE(bytes(field(decoded.value, "payload")) == std::vector<uint8_t>{0xDE, 0xAD, 0xBE, 0xEF});
}
