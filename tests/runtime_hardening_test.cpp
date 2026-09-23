#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "runtime/Runtime.h"
#include "encoder/Encoder.h"
#include "diagnostics/Diagnostic.h"
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

using namespace embx;

static std::vector<uint8_t> encodeBitsBig(uint64_t value, size_t totalBits) {
    const size_t bytes = (totalBits + 7) / 8;
    const size_t padding = bytes * 8 - totalBits;
    uint64_t container = value << padding;
    std::vector<uint8_t> out(bytes);
    for (size_t i = 0; i < bytes; ++i)
        out[bytes - 1 - i] = static_cast<uint8_t>(container >> (8 * i));
    return out;
}

static std::vector<uint8_t> encodeBitsLittle(uint64_t value, size_t totalBits) {
    const size_t bytes = (totalBits + 7) / 8;
    const size_t padding = bytes * 8 - totalBits;
    uint64_t container = value << padding;
    std::vector<uint8_t> out(bytes);
    for (size_t i = 0; i < bytes; ++i)
        out[i] = static_cast<uint8_t>(container >> (8 * i));
    return out;
}

TEST_CASE("bit container matrix 1..64", "[runtime_hardening][bits]") {
    for (size_t width = 1; width <= 64; ++width) {
        const uint64_t mask = width == 64 ? std::numeric_limits<uint64_t>::max() : ((uint64_t{1} << width) - 1);
        const uint64_t value = width == 64 ? UINT64_C(0xA55AA55AA55AA55A) : ((UINT64_C(0xA55AA55AA55AA55A) ^ (UINT64_C(0x9E3779B97F4A7C15) >> (64 - width))) & mask);

        const auto bigData = encodeBitsBig(value, width);
        runtime::Reader big(bigData);
        big.endian(runtime::Endian::Big);
        big.beginBits(width);
        const auto bigGot = big.bits(width);
        REQUIRE(bigGot == value);
        big.endBits();
        REQUIRE(big.pos() == (width + 7) / 8);

        const auto littleData = encodeBitsLittle(value, width);
        runtime::Reader little(littleData);
        little.endian(runtime::Endian::Little);
        little.beginBits(width);
        const auto littleGot = little.bits(width);
        REQUIRE(littleGot == value);
        little.endBits();
        REQUIRE(little.pos() == (width + 7) / 8);
    }
}

TEST_CASE("callback rollback on exception", "[runtime_hardening][callback]") {
    std::vector<uint8_t> data{1, 2, 3, 4};
    runtime::Reader r(data);
    runtime::CallbackRegistry registry;
    registry.add("throws", [](const std::string&, runtime::Reader& rr, const runtime::Environment&, const std::vector<runtime::Value>&, std::string&) -> bool {
        rr.seek(4);
        throw std::runtime_error("boom");
    });

    r.seek(1);
    auto result = registry.call("throws", r);
    REQUIRE(result.invoked);
    REQUIRE(!result.success);
    REQUIRE(result.error == "boom");
    REQUIRE(r.pos() == 1);
}

TEST_CASE("decode callback rollback restores the complete reader state", "[runtime_hardening][callback]") {
    std::vector<uint8_t> data{1, 2, 3, 4};
    runtime::Reader r(data);
    r.endian(runtime::Endian::Big);
    r.seek(1);

    runtime::CallbackRegistry registry;
    registry.add("mutates", [](const std::string&, runtime::Reader& rr, const runtime::Environment&, const std::vector<runtime::Value>&, std::string&) -> bool {
        rr.endian(runtime::Endian::Little);
        rr.pushLimit(2);
        rr.beginBits(8);
        (void)rr.bits(4);
        return false;
    });

    const auto before = r.state();
    auto result = registry.call("mutates", r);
    REQUIRE(result.invoked);
    REQUIRE(!result.success);
    const auto after = r.state();
    REQUIRE(after.pos == before.pos);
    REQUIRE(after.limit == before.limit);
    REQUIRE(after.endian == before.endian);
    REQUIRE(after.limits == before.limits);
    REQUIRE(after.bitContainer == before.bitContainer);
    REQUIRE(after.bitRemaining == before.bitRemaining);
}

TEST_CASE("diagnostic classification is stable", "[runtime_hardening][diagnostics]") {
    using diagnostics::Code;
    REQUIRE(diagnostics::classify("unknown struct: X") == Code::UnknownStruct);
    REQUIRE(diagnostics::classify("unknown named type: T") == Code::UnknownType);
    REQUIRE(diagnostics::classify("missing field: x") == Code::MissingField);
    REQUIRE(diagnostics::classify("no variant case for discriminator") == Code::VariantMismatch);
    REQUIRE(diagnostics::classify("block members exceed block size") == Code::BlockConstraint);
    REQUIRE(diagnostics::classify("negative size") == Code::ExpressionError);
    REQUIRE(diagnostics::classify("integer read beyond reader limit") == Code::InputTruncated);
    REQUIRE(diagnostics::classify("encode callback registry is not configured: cb", true) == Code::OutputConstraint);

    auto d = diagnostics::make(Code::OutOfRange, "length mismatch", "Packet.payload", 7, 4, 3, true);
    REQUIRE(diagnostics::format(d) == "out_of_range: Packet.payload: length mismatch [expected=4, actual=3] @byte 7");
}

TEST_CASE("encoder enforces array and recursion limits", "[runtime_hardening][encoder]") {
    plan::Module p;
    plan::Struct s;
    s.name = "Packet";

    auto arr = std::make_unique<plan::Field>();
    arr->name = "values";
    arr->type.kind = core::TypeKind::Primitive;
    arr->type.name = "u8";
    auto expr = std::make_unique<plan::Expr>();
    expr->kind = core::ExprKind::Literal;
    expr->text = "4";
    arr->type.dimensions.push_back(embx::core::Dimension::fixed(std::move(expr)));
    s.members.push_back(std::move(arr));

    embx::test::addStruct(p, std::move(s));

    value::Value::Array values;
    values.emplace_back(uint64_t(1));
    values.emplace_back(uint64_t(2));
    values.emplace_back(uint64_t(3));
    values.emplace_back(uint64_t(4));

    encoder::Options arrayOptions;
    arrayOptions.maxArrayElements = 2;
    auto arrayResult = encoder::Engine(p, arrayOptions).encode("Packet", value::Value(value::Value::Object{{"values", value::Value(values)}}));
    REQUIRE(!arrayResult.success);
    REQUIRE(arrayResult.diagnostic.code == diagnostics::Code::OutOfRange);

    encoder::Options allocationOptions;
    allocationOptions.maxArrayElements = 16;
    allocationOptions.maxAllocationBytes = sizeof(value::Value) * 2;
    auto allocationResult = encoder::Engine(p, allocationOptions).encode("Packet", value::Value(value::Value::Object{{"values", value::Value(values)}}));
    REQUIRE(!allocationResult.success);
    REQUIRE(allocationResult.diagnostic.code == diagnostics::Code::OutOfRange);

    plan::Module recursive;
    plan::Struct node;
    node.name = "Node";
    const auto nodeId = embx::test::addStruct(recursive, std::move(node));
    auto child = std::make_unique<plan::Field>();
    child->name = "child";
    child->type.kind = core::TypeKind::Named;
    child->type.name = "Node";
    child->type.reference.id = nodeId;
    embx::test::addFieldSymbol(recursive, *child);
    recursive.structs.front().members.push_back(std::move(child));

    encoder::Options recursionOptions;
    recursionOptions.maxRecursionDepth = 1;
    value::Value::Object nested{{"child", value::Value(value::Value::Object{})}};
    auto recursionResult = encoder::Engine(recursive, recursionOptions).encode("Node", value::Value(nested));
    REQUIRE(!recursionResult.success);
    REQUIRE((recursionResult.diagnostic.code == diagnostics::Code::OutOfRange ||
             recursionResult.diagnostic.code == diagnostics::Code::OutputConstraint));
}

TEST_CASE("reader rejects malformed restored state") {
    std::vector<uint8_t> data(16, 0);
    runtime::Reader r(data);

    auto badPosition = r.state();
    badPosition.pos = 17;
    REQUIRE_THROWS_AS(r.restore(badPosition), std::out_of_range);

    auto badBits = r.state();
    badBits.bitRemaining = 65;
    REQUIRE_THROWS_AS(r.restore(badBits), std::out_of_range);

    auto badLimits = r.state();
    badLimits.limit = 8;
    badLimits.limits = {12, 16};
    REQUIRE_THROWS_AS(r.restore(badLimits), std::out_of_range);

    auto badNestedPosition = r.state();
    badNestedPosition.limit = 8;
    badNestedPosition.limits = {16};
    badNestedPosition.pos = 9;
    REQUIRE_THROWS_AS(r.restore(badNestedPosition), std::out_of_range);

    auto badRootLimit = r.state();
    badRootLimit.limit = 8;
    badRootLimit.limits.clear();
    REQUIRE_THROWS_AS(r.restore(badRootLimit), std::out_of_range);

    r.seek(8);
    REQUIRE_THROWS_AS(r.pushLimitEnd(7), std::out_of_range);
    REQUIRE(r.limit() == data.size());
    REQUIRE(r.pos() == 8);
}
