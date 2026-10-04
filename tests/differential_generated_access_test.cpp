#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "value/Value.h"
#include "generated.hpp"

namespace {
std::shared_ptr<embx::plan::Module> makePlan(std::string& error) {
    auto ast = embx::parser::parseFile("../tests/fixtures/differential_access.embx", error);
    if (!ast) return {};
    if (!embx::semantic::analyze(*ast, error)) return {};
    auto ir = embx::ir::lower(*ast, error);
    if (!ir) return {};
    auto plan = embx::plan::build(*ir, error);
    if (!plan) return {};
    return std::shared_ptr<embx::plan::Module>(std::move(plan));
}

embx::encoder::Result referenceEncode(const embx::plan::Module& plan,
                                      const std::string& type,
                                      const embx::value::Value::Object& value) {
    return embx::encoder::Engine(plan).encode(type, value);
}

TEST_CASE("generated C++ matches Reference Runtime for virtual field and alias") {
    std::string error;
    auto plan = makePlan(error);
    REQUIRE(plan);
    REQUIRE(error.empty());

    embx::value::Value::Object input;
    input["count"] = std::uint64_t(2);
    input["payload"] = embx::value::Value::Bytes{0x11, 0x22, 0x33, 0x44};

    const auto referenceResult = referenceEncode(*plan, "differential_access::AccessPacket", input);
    REQUIRE(referenceResult);
    const auto& reference = referenceResult.data;
    REQUIRE(reference == std::vector<std::uint8_t>{0x02, 0x11, 0x22, 0x33, 0x44});

    differential_access::AccessPacket generated{};
    generated.count = 2;
    generated.payload = {0x11, 0x22, 0x33, 0x44};

    std::vector<std::uint8_t> generatedBytes;
    REQUIRE(differential_access::encode__differential_access__AccessPacket(generated, generatedBytes));
    REQUIRE(generatedBytes == reference);
    REQUIRE(generated.doubled == 4);

    auto copied = generated;
    REQUIRE(copied.doubled == 4);
    generated.count = 3;
    REQUIRE(generated.doubled == 6);
    REQUIRE(copied.doubled == 4);

    auto moved = std::move(copied);
    REQUIRE(moved.doubled == 4);
    moved.count = 5;
    REQUIRE(moved.doubled == 10);
    REQUIRE(generated.count_alias() == 3);

    differential_access::AccessPacket decoded{};
    std::size_t consumed = 0;
    REQUIRE(differential_access::decode__differential_access__AccessPacket(reference, decoded, consumed));
    REQUIRE(consumed == reference.size());
    REQUIRE(decoded.count == 2);
    REQUIRE(decoded.doubled == 4);
    REQUIRE(decoded.count_alias() == 2);
    REQUIRE(decoded.payload == std::vector<std::uint8_t>{0x11, 0x22, 0x33, 0x44});
}

TEST_CASE("generated C++ alias setter matches Reference Runtime") {
    std::string error;
    auto plan = makePlan(error);
    REQUIRE(plan);
    REQUIRE(error.empty());

    embx::value::Value::Object input;
    input["count"] = std::uint64_t(3);
    input["payload"] = embx::value::Value::Bytes{1, 2, 3, 4, 5, 6};
    const auto referenceResult = referenceEncode(*plan, "differential_access::AccessPacket", input);
    REQUIRE(referenceResult);
    const auto& reference = referenceResult.data;

    differential_access::AccessPacket generated{};
    generated.count_alias() = 3;
    generated.payload = {1, 2, 3, 4, 5, 6};

    std::vector<std::uint8_t> generatedBytes;
    REQUIRE(differential_access::encode__differential_access__AccessPacket(generated, generatedBytes));
    REQUIRE(generatedBytes == reference);
}

TEST_CASE("generated C++ matches Reference Runtime for scale transform") {
    std::string error;
    auto plan = makePlan(error);
    REQUIRE(plan);
    REQUIRE(error.empty());

    embx::value::Value::Object input;
    input["raw"] = 25.3;
    const auto referenceResult = referenceEncode(*plan, "differential_access::TransformPacket", input);
    REQUIRE(referenceResult);
    const auto& reference = referenceResult.data;
    REQUIRE(reference == std::vector<std::uint8_t>{0x00, 0xFD});

    differential_access::TransformPacket generated{};
    generated.raw = 25.3;

    std::vector<std::uint8_t> generatedBytes;
    REQUIRE(differential_access::encode__differential_access__TransformPacket(generated, generatedBytes));
    REQUIRE(generatedBytes == reference);

    differential_access::TransformPacket decoded{};
    std::size_t consumed = 0;
    REQUIRE(differential_access::decode__differential_access__TransformPacket(reference, decoded, consumed));
    REQUIRE(consumed == reference.size());
    REQUIRE(decoded.raw == Catch::Approx(25.3));
}


TEST_CASE("generated C++ matches Reference Runtime for negative scale transform") {
    std::string error;
    auto plan = makePlan(error);
    REQUIRE(plan);
    REQUIRE(error.empty());

    embx::value::Value::Object input;
    input["raw"] = -12.5;
    const auto referenceResult = referenceEncode(*plan, "differential_access::TransformPacket", input);
    REQUIRE(referenceResult);
    REQUIRE(referenceResult.data.size() == 2);

    differential_access::TransformPacket generated{};
    generated.raw = -12.5;
    std::vector<std::uint8_t> generatedBytes;
    REQUIRE(differential_access::encode__differential_access__TransformPacket(generated, generatedBytes));
    REQUIRE(generatedBytes == referenceResult.data);

    differential_access::TransformPacket decoded{};
    std::size_t consumed = 0;
    REQUIRE(differential_access::decode__differential_access__TransformPacket(referenceResult.data, decoded, consumed));
    REQUIRE(consumed == referenceResult.data.size());
    REQUIRE(decoded.raw == Catch::Approx(-12.5));
}

TEST_CASE("generated C++ matches Reference Runtime for rejected scale rounding") {
    std::string error;
    auto plan = makePlan(error);
    REQUIRE(plan);
    REQUIRE(error.empty());

    embx::value::Value::Object input;
    input["raw"] = 25.35;
    const auto referenceResult = referenceEncode(*plan, "differential_access::TransformPacket", input);
    REQUIRE_FALSE(referenceResult);

    differential_access::TransformPacket generated{};
    generated.raw = 25.35;
    std::vector<std::uint8_t> generatedBytes{0xAA};
    std::string generatedError;
    REQUIRE_FALSE(differential_access::encode__differential_access__TransformPacket(
        generated, generatedBytes, nullptr, &generatedError));
    REQUIRE(generatedBytes.empty());
}


TEST_CASE("generated C++ matches Reference Runtime for floating-point scale transforms") {
    std::string error;
    auto plan = makePlan(error);
    REQUIRE(plan);
    REQUIRE(error.empty());

    embx::value::Value::Object input;
    input["f32Value"] = 12.5;
    input["f64Value"] = -25.0;
    const auto referenceResult = referenceEncode(
        *plan, "differential_access::FloatTransformPacket", input);
    REQUIRE(referenceResult);

    differential_access::FloatTransformPacket generated{};
    generated.f32Value = 12.5;
    generated.f64Value = -25.0;

    std::vector<std::uint8_t> generatedBytes;
    REQUIRE(differential_access::encode__differential_access__FloatTransformPacket(
        generated, generatedBytes));
    REQUIRE(generatedBytes == referenceResult.data);

    differential_access::FloatTransformPacket decoded{};
    std::size_t consumed = 0;
    REQUIRE(differential_access::decode__differential_access__FloatTransformPacket(
        referenceResult.data, decoded, consumed));
    REQUIRE(consumed == referenceResult.data.size());
    REQUIRE(decoded.f32Value == Catch::Approx(12.5));
    REQUIRE(decoded.f64Value == Catch::Approx(-25.0));
}

} // namespace
