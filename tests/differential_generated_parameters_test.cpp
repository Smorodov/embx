#include <catch2/catch_test_macros.hpp>
#include "compiler/Compiler.h"
#include "generated.hpp"
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

static std::uint64_t number(const Value& v) {
    return std::get<std::uint64_t>(v.data);
}

static std::unique_ptr<embx::compiler::Compilation> compileParameters(std::string& error) {
    return embx::compiler::compileFile("../tests/fixtures/differential_parameters.embx", error);
}

TEST_CASE("generated C++ matches Reference Runtime for runtime parameters") {
    std::string error;
    auto c = compileParameters(error);
    REQUIRE(c);
    REQUIRE(error.empty());

    const auto count = c->plan->symbolTable.findId("differential_parameters::count");
    const auto delta = c->plan->symbolTable.findId("differential_parameters::delta");
    REQUIRE(count != embx::core::InvalidSymbolId);
    REQUIRE(delta != embx::core::InvalidSymbolId);

    Object root{
        {"header", uint64_t{0xA1}},
        {"scaled", 12.5},
        {"values", Value{Array{Value{uint64_t{0x11}}, Value{uint64_t{0x22}}, Value{uint64_t{0x33}}}}},
        {"marker", uint64_t{0xA5}},
        {"tail", uint64_t{0x7E}}
    };

    embx::encoder::Options eo;
    eo.parameters[count] = uint64_t{3};
    eo.parameters[delta] = uint64_t{7};
    auto ref = embx::encoder::Engine(*c->plan, eo).encode("differential_parameters::RuntimePacket", Value{root});
    REQUIRE(ref);
    REQUIRE(ref.data == std::vector<std::uint8_t>{0xA1, 0x00, 0x19, 0x11, 0x22, 0x33, 0x7E, 0xA5});

    differential_parameters::RuntimePacket generatedValue{};
    generatedValue.header = 0xA1;
    generatedValue.scaled = 12.5;
    generatedValue.values = {0x11, 0x22, 0x33};
    generatedValue.marker = 0xA5;
    generatedValue.tail = 0x7E;

    // Regression guard: every validated Plan parameter must be present in the generated ABI.
    embx_generated_detail::RuntimeParameters params{};
    params.count = 3;
    params.delta = 7;

    std::vector<std::uint8_t> generatedBytes;
    REQUIRE(differential_parameters::encode__differential_parameters__RuntimePacket(
        generatedValue, generatedBytes, params));
    REQUIRE(generatedBytes == ref.data);

    embx::decoder::Options dopt;
    dopt.parameters[count] = uint64_t{3};
    dopt.parameters[delta] = uint64_t{7};
    dopt.requireFullInput = false;
    auto decoded = embx::decoder::Engine(*c->plan, dopt).decode(
        "differential_parameters::RuntimePacket", ref.data);
    REQUIRE(decoded);
    REQUIRE(decoded.consumed == 7);

    differential_parameters::RuntimePacket generatedDecoded{};
    std::size_t consumed = 0;
    REQUIRE(differential_parameters::decode__differential_parameters__RuntimePacket(
        ref.data, generatedDecoded, consumed, params));
    REQUIRE(consumed == 7);
    REQUIRE(generatedDecoded.header == number(field(decoded.value, "header")));
    REQUIRE(generatedDecoded.scaled == std::get<double>(field(decoded.value, "scaled").data));
    REQUIRE(generatedDecoded.values.size() == 3);
    REQUIRE(generatedDecoded.values[0] == 0x11);
    REQUIRE(generatedDecoded.values[2] == 0x33);
    REQUIRE(generatedDecoded.marker == number(field(decoded.value, "marker")));
    REQUIRE(generatedDecoded.tail == number(field(decoded.value, "tail")));
}

TEST_CASE("generated C++ uses independent runtime parameter values") {
    std::string error;
    auto c = compileParameters(error);
    REQUIRE(c);
    REQUIRE(error.empty());

    const auto count = c->plan->symbolTable.findId("differential_parameters::count");
    const auto delta = c->plan->symbolTable.findId("differential_parameters::delta");
    REQUIRE(count != embx::core::InvalidSymbolId);
    REQUIRE(delta != embx::core::InvalidSymbolId);

    Object root{
        {"header", uint64_t{0xB2}},
        {"scaled", -12.5},
        {"values", Value{Array{Value{uint64_t{0x44}}, Value{uint64_t{0x55}}}}},
        {"marker", uint64_t{0xC6}},
        {"tail", uint64_t{0x7F}}
    };

    embx::encoder::Options eo;
    eo.parameters[count] = uint64_t{2};
    eo.parameters[delta] = uint64_t{6};
    auto ref = embx::encoder::Engine(*c->plan, eo).encode("differential_parameters::RuntimePacket", Value{root});
    REQUIRE(ref);
    REQUIRE(ref.data == std::vector<std::uint8_t>{0xB2, 0xFF, 0xE7, 0x44, 0x55, 0x7F, 0xC6});

    differential_parameters::RuntimePacket generatedValue{};
    generatedValue.header = 0xB2;
    generatedValue.scaled = -12.5;
    generatedValue.values = {0x44, 0x55};
    generatedValue.marker = 0xC6;
    generatedValue.tail = 0x7F;
    embx_generated_detail::RuntimeParameters params{};
    params.count = 2;
    params.delta = 6;

    std::vector<std::uint8_t> generatedBytes;
    REQUIRE(differential_parameters::encode__differential_parameters__RuntimePacket(
        generatedValue, generatedBytes, params));
    REQUIRE(generatedBytes == ref.data);
}
