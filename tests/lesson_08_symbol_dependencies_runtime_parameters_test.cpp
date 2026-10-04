#include <catch2/catch_test_macros.hpp>
#include "compiler/Compiler.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "value/Value.h"
#include "core/Symbol.h"
#include <cstdint>
#include <vector>

using Value = embx::value::Value;
using Object = Value::Object;
using Array = Value::Array;

static uint64_t number(const Value& v) {
    return std::get<uint64_t>(v.data);
}

TEST_CASE("Lesson 8 parameters have canonical symbols and drive executable expressions") {
    std::string error;
    auto compilation = embx::compiler::compileFile(
        "../course/08_symbol_dependencies_runtime_parameters/parameters.embx", error);
    REQUIRE(compilation);
    REQUIRE(error.empty());
    REQUIRE(compilation->plan);
    REQUIRE(compilation->plan->parameters.size() == 2);

    const auto count = compilation->plan->symbolTable.findId("count");
    const auto gap = compilation->plan->symbolTable.findId("gap");
    REQUIRE(count != embx::core::InvalidSymbolId);
    REQUIRE(gap != embx::core::InvalidSymbolId);
    REQUIRE(count != gap);
    REQUIRE(compilation->plan->symbolTable.find(count)->kind == embx::core::SymbolKind::Parameter);
    REQUIRE(compilation->plan->symbolTable.find(gap)->kind == embx::core::SymbolKind::Parameter);
    REQUIRE(compilation->plan->parameterIndexBySymbol.count(count) == 1);
    REQUIRE(compilation->plan->parameterIndexBySymbol.count(gap) == 1);

    embx::encoder::Options encodeOptions;
    encodeOptions.parameters[count] = uint64_t{3};
    encodeOptions.parameters[gap] = uint64_t{1};
    embx::encoder::Engine encoder(*compilation->plan, encodeOptions);

    Object input{
        {"values", Value{Array{Value{uint64_t{0x11}}, Value{uint64_t{0x22}}, Value{uint64_t{0x33}}}}},
        {"marker", Value{uint64_t{0xAA}}}
    };
    const auto encoded = encoder.encode("ParameterPacket", Value{input});
    REQUIRE(encoded.success);
    REQUIRE(encoded.data == std::vector<uint8_t>{0x11, 0x22, 0x33, 0x00, 0xAA});

    embx::decoder::Options decodeOptions;
    decodeOptions.parameters[count] = uint64_t{3};
    decodeOptions.parameters[gap] = uint64_t{1};
    embx::decoder::Engine decoder(*compilation->plan, decodeOptions);
    const auto decoded = decoder.decode("ParameterPacket", encoded.data);
    REQUIRE(decoded.success);

    const auto& object = std::get<Object>(decoded.value.data);
    REQUIRE(std::get<Array>(object.at("values").data).size() == 3);
    REQUIRE(number(object.at("marker")) == 0xAA);
}
