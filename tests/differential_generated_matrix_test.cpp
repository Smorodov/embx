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

static std::unique_ptr<embx::compiler::Compilation> compileMatrix(std::string& error) {
    return embx::compiler::compileFile("../tests/fixtures/differential_matrix.embx", error);
}

TEST_CASE("generated C++ matches Reference Runtime for explicit byte order") {
    std::string error;
    auto c = compileMatrix(error);
    REQUIRE(c);
    REQUIRE(error.empty());
    Object root{{"bigValue", uint64_t{0x1234}}, {"littleValue", uint64_t{0x5678}},
                {"signedLittle", int64_t{-2}}, {"byteValue", uint64_t{0xA5}}};
    auto ref = embx::encoder::Engine(*c->plan).encode("differential_matrix::Numbers", Value{root});
    REQUIRE(ref);
    differential_matrix::Numbers generatedValue{0x1234, 0x5678, -2, 0xA5};
    std::vector<std::uint8_t> generatedBytes;
    REQUIRE(differential_matrix::encode__differential_matrix__Numbers(generatedValue, generatedBytes));
    REQUIRE(generatedBytes == ref.data);

    embx::decoder::Options options; options.requireFullInput = true;
    auto decoded = embx::decoder::Engine(*c->plan, options).decode("differential_matrix::Numbers", ref.data);
    REQUIRE(decoded);
    differential_matrix::Numbers generatedDecoded{}; std::size_t consumed = 0;
    REQUIRE(differential_matrix::decode__differential_matrix__Numbers(ref.data, generatedDecoded, consumed));
    REQUIRE(consumed == ref.data.size());
    REQUIRE(generatedDecoded.bigValue == number(field(decoded.value, "bigValue")));
    REQUIRE(generatedDecoded.littleValue == number(field(decoded.value, "littleValue")));
    REQUIRE(generatedDecoded.signedLittle == std::get<std::int64_t>(field(decoded.value, "signedLittle").data));
    REQUIRE(generatedDecoded.byteValue == number(field(decoded.value, "byteValue")));
}

TEST_CASE("generated C++ matches Reference Runtime for bit fields") {
    std::string error;
    auto c = compileMatrix(error);
    REQUIRE(c);
    REQUIRE(error.empty());
    Object root{{"version", uint64_t{5}}, {"kind", uint64_t{2}}, {"enabled", uint64_t{1}}, {"code", uint64_t{1}}};
    auto ref = embx::encoder::Engine(*c->plan).encode("differential_matrix::Flags", Value{root});
    REQUIRE(ref);
    differential_matrix::Flags generatedValue{}; generatedValue.version=5; generatedValue.kind=2; generatedValue.enabled=1; generatedValue.code=1;
    std::vector<std::uint8_t> generatedBytes;
    REQUIRE(differential_matrix::encode__differential_matrix__Flags(generatedValue, generatedBytes));
    REQUIRE(generatedBytes == ref.data);

    embx::decoder::Options options; options.requireFullInput=true;
    auto decoded=embx::decoder::Engine(*c->plan,options).decode("differential_matrix::Flags",ref.data);
    REQUIRE(decoded);
    differential_matrix::Flags generatedDecoded{}; std::size_t consumed=0;
    REQUIRE(differential_matrix::decode__differential_matrix__Flags(ref.data,generatedDecoded,consumed));
    REQUIRE(consumed==ref.data.size());
    REQUIRE(generatedDecoded.version == number(field(decoded.value,"version")));
    REQUIRE(generatedDecoded.kind == number(field(decoded.value,"kind")));
    REQUIRE(generatedDecoded.enabled == number(field(decoded.value,"enabled")));
    REQUIRE(generatedDecoded.code == number(field(decoded.value,"code")));
}

TEST_CASE("generated C++ matches Reference Runtime for fixed arrays") {
    std::string error;
    auto c = compileMatrix(error);
    REQUIRE(c);
    REQUIRE(error.empty());
    Object root{{"values", Value{Array{Value{uint64_t{1}},Value{uint64_t{2}},Value{uint64_t{3}},Value{uint64_t{4}}}}}};
    auto ref = embx::encoder::Engine(*c->plan).encode("differential_matrix::StaticArray", Value{root});
    REQUIRE(ref);
    differential_matrix::StaticArray generatedValue{}; generatedValue.values = {1,2,3,4};
    std::vector<std::uint8_t> generatedBytes;
    REQUIRE(differential_matrix::encode__differential_matrix__StaticArray(generatedValue, generatedBytes));
    REQUIRE(generatedBytes == ref.data);

    embx::decoder::Options options; options.requireFullInput=true;
    auto decoded=embx::decoder::Engine(*c->plan,options).decode("differential_matrix::StaticArray",ref.data);
    REQUIRE(decoded);
    differential_matrix::StaticArray generatedDecoded{}; std::size_t consumed=0;
    REQUIRE(differential_matrix::decode__differential_matrix__StaticArray(ref.data,generatedDecoded,consumed));
    REQUIRE(consumed==ref.data.size());
    REQUIRE(generatedDecoded.values[0]==1); REQUIRE(generatedDecoded.values[3]==4);
}

TEST_CASE("generated C++ matches Reference Runtime for nested structures") {
    std::string error;
    auto c=compileMatrix(error);
    REQUIRE(c);
    REQUIRE(error.empty());
    Object header{{"version",uint64_t{2}}, {"flags",uint64_t{0xA0}}};
    Object root{{"header",Value{header}}, {"sequence",uint64_t{0x1234}},
                {"payload",Value{Array{Value{uint64_t{0x11}},Value{uint64_t{0x22}},Value{uint64_t{0x33}}}}}};
    auto ref=embx::encoder::Engine(*c->plan).encode("differential_matrix::NestedPacket",Value{root}); REQUIRE(ref);
    differential_matrix::NestedPacket generatedValue{}; generatedValue.header.version=2; generatedValue.header.flags=0xA0;
    generatedValue.sequence=0x1234; generatedValue.payload={0x11,0x22,0x33};
    std::vector<std::uint8_t> generatedBytes; REQUIRE(differential_matrix::encode__differential_matrix__NestedPacket(generatedValue,generatedBytes));
    REQUIRE(generatedBytes==ref.data);

    embx::decoder::Options options; options.requireFullInput=true;
    auto decoded=embx::decoder::Engine(*c->plan,options).decode("differential_matrix::NestedPacket",ref.data); REQUIRE(decoded);
    differential_matrix::NestedPacket generatedDecoded{}; std::size_t consumed=0; REQUIRE(differential_matrix::decode__differential_matrix__NestedPacket(ref.data,generatedDecoded,consumed));
    REQUIRE(consumed==ref.data.size());
    REQUIRE(generatedDecoded.header.version==number(field(field(decoded.value,"header"),"version")));
    REQUIRE(generatedDecoded.sequence==number(field(decoded.value,"sequence")));
    REQUIRE(generatedDecoded.payload[2]==0x33);
}

TEST_CASE("generated C++ matches Reference Runtime for variants") {
    std::string error;
    auto c=compileMatrix(error);
    REQUIRE(c);
    REQUIRE(error.empty());
    Object root{{"kind",uint64_t{1}}, {"body",Value{Object{{"value",uint64_t{0x1234}}}}}};
    auto ref=embx::encoder::Engine(*c->plan).encode("differential_matrix::VariantPacket",Value{root}); REQUIRE(ref);
    differential_matrix::VariantPacket generatedValue{}; generatedValue.kind=1; generatedValue.body.value.template emplace<0>(static_cast<std::uint16_t>(0x1234));
    std::vector<std::uint8_t> generatedBytes; REQUIRE(differential_matrix::encode__differential_matrix__VariantPacket(generatedValue,generatedBytes));
    REQUIRE(generatedBytes==ref.data);

    embx::decoder::Options options; options.requireFullInput=true;
    auto decoded=embx::decoder::Engine(*c->plan,options).decode("differential_matrix::VariantPacket",ref.data); REQUIRE(decoded);
    differential_matrix::VariantPacket generatedDecoded{}; std::size_t consumed=0; REQUIRE(differential_matrix::decode__differential_matrix__VariantPacket(ref.data,generatedDecoded,consumed));
    REQUIRE(consumed==ref.data.size());
    REQUIRE(generatedDecoded.kind==number(field(decoded.value,"kind")));
    REQUIRE(std::get<0>(generatedDecoded.body.value)==static_cast<std::uint16_t>(0x1234));
}
