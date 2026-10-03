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

static const Value& field(const Value& v, const char* name) {
    return std::get<Object>(v.data).at(name);
}

TEST_CASE("generated C++ backend is encode-conformant with Reference Runtime") {
    std::string error;
    auto compilation = embx::compiler::compileFile("../examples/common.embx", error);
    REQUIRE(compilation);
    REQUIRE(error.empty());

    Object root;
    root["magic"] = Value{uint64_t{0xA1B2}};
    root["kind"] = Value{uint64_t{0x42}};

    const auto reference = embx::encoder::Engine(*compilation->plan).encode("common::net::Header", Value{root});
    REQUIRE(reference.success);

    common::net::Header generatedValue{};
    generatedValue.magic = 0xA1B2;
    generatedValue.kind = 0x42;
    std::vector<std::uint8_t> generatedBytes;
    REQUIRE(common::net::encode__common__net__Header(generatedValue, generatedBytes));

    REQUIRE(generatedBytes == reference.data);
    REQUIRE(generatedBytes == std::vector<std::uint8_t>{0xA1, 0xB2, 0x42});
}

TEST_CASE("generated C++ backend is decode-conformant with Reference Runtime") {
    std::string error;
    auto compilation = embx::compiler::compileFile("../examples/common.embx", error);
    REQUIRE(compilation);
    REQUIRE(error.empty());

    const std::vector<std::uint8_t> bytes{0xA1, 0xB2, 0x42};

    embx::decoder::Options options;
    options.requireFullInput = true;
    const auto reference = embx::decoder::Engine(*compilation->plan, options).decode("common::net::Header", bytes);
    REQUIRE(reference.success);

    common::net::Header generatedValue{};
    std::size_t consumed = 0;
    std::string generatedError;
    REQUIRE(common::net::decode__common__net__Header(bytes, generatedValue, consumed, nullptr, &generatedError));
    REQUIRE(generatedError.empty());
    REQUIRE(consumed == bytes.size());

    REQUIRE(generatedValue.magic == std::get<std::uint64_t>(field(reference.value, "magic").data));
    REQUIRE(generatedValue.kind == std::get<std::uint64_t>(field(reference.value, "kind").data));
}
