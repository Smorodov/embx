#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "compiler/Compiler.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "generated.hpp"

namespace {
std::shared_ptr<embx::plan::Module> makePlan(std::string& error) {
    auto compilation = embx::compiler::compileFile("../tests/fixtures/differential_callbacks.embx", error);
    if (!compilation) return {};
    return std::shared_ptr<embx::plan::Module>(std::move(compilation->plan));
}
}

TEST_CASE("generated C++ matches Reference Runtime for encode callbacks") {
    std::string error;
    auto plan = makePlan(error);
    REQUIRE(plan);
    REQUIRE(error.empty());

    embx::runtime::CallbackRegistry referenceCallbacks;
    referenceCallbacks.add("differential_callbacks::on_encode_event", [](const std::string&, std::vector<std::uint8_t>& out,
                                                   std::size_t& pos, const embx::runtime::Environment&,
                                                   const std::vector<embx::runtime::Value>& args, std::string& callbackError) {
        if (args.size() != 1 || !std::holds_alternative<std::uint64_t>(args[0]) ||
            std::get<std::uint64_t>(args[0]) != 0x2A) {
            callbackError = "unexpected encode callback argument"; return false;
        }
        if (pos >= out.size()) out.resize(pos + 1);
        out[pos++] = 0xA5;
        return true;
    });

    embx::encoder::Options options;
    options.callbacks = &referenceCallbacks;
    embx::value::Value::Object input;
    input["value"] = std::uint64_t{0x2A};
    const auto reference = embx::encoder::Engine(*plan, options).encode(
        "differential_callbacks::EncodeCallbackPacket", input);
    INFO("Reference encode error: " << reference.error);
    REQUIRE(reference);
    REQUIRE(reference.data == std::vector<std::uint8_t>{0x2A, 0xA5});

    differential_callbacks::EncodeCallbackPacket generated{};
    generated.value = 0x2A;
    embx_generated_detail::Callbacks generatedCallbacks;
    REQUIRE(generatedCallbacks.addEncode(
        "differential_callbacks::on_encode_event",
        [](const std::string&, embx_generated_detail::Writer& w,
           const std::nullptr_t&, const std::vector<embx_generated_detail::CallbackValue>& args,
           std::string& callbackError) {
            if (args.size() != 1 || !std::holds_alternative<std::uint64_t>(args[0]) ||
                std::get<std::uint64_t>(args[0]) != 0x2A) {
                callbackError = "unexpected encode callback argument";
                return false;
            }
            if (!w.ensure(1)) { callbackError = "output overflow"; return false; }
            w.d[w.p] = 0xA5;
            ++w.p;
            return true;
        }));

    std::vector<std::uint8_t> generatedBytes;
    std::string generatedError;
    REQUIRE(differential_callbacks::encode__differential_callbacks__EncodeCallbackPacket(
        generated, generatedBytes, &generatedCallbacks, &generatedError));
    REQUIRE(generatedError.empty());
    REQUIRE(generatedBytes == reference.data);
}

TEST_CASE("generated C++ matches Reference Runtime for decode callbacks") {
    std::string error;
    auto plan = makePlan(error);
    REQUIRE(plan);
    REQUIRE(error.empty());

    const std::vector<std::uint8_t> wire{0x2A, 0xA5};

    embx::runtime::CallbackRegistry referenceCallbacks;
    referenceCallbacks.add("differential_callbacks::on_decode_event", [](const std::string&, embx::runtime::Reader& reader,
                                                   const embx::runtime::Environment&,
                                                   const std::vector<embx::runtime::Value>& args, std::string& callbackError) {
        if (args.size() != 1 || !std::holds_alternative<std::uint64_t>(args[0]) ||
            std::get<std::uint64_t>(args[0]) != 0x2A) {
            callbackError = "unexpected decode callback argument"; return false;
        }
        if (reader.remaining() == 0 || reader.uint(1) != 0xA5) {
            callbackError = "unexpected callback marker"; return false;
        }
        return true;
    });

    embx::decoder::Engine decoder(*plan);
    decoder.callbacks(&referenceCallbacks);
    const auto reference = decoder.decode("differential_callbacks::DecodeCallbackPacket", wire);
    INFO("Reference decode error: " << reference.error);
    REQUIRE(reference);
    REQUIRE(reference.consumed == wire.size());

    differential_callbacks::DecodeCallbackPacket generated{};
    std::size_t consumed = 0;
    embx_generated_detail::Callbacks generatedCallbacks;
    REQUIRE(generatedCallbacks.addDecode(
        "differential_callbacks::on_decode_event",
        [](const std::string&, embx_generated_detail::Reader& reader,
           const std::nullptr_t&, const std::vector<embx_generated_detail::CallbackValue>& args,
           std::string& callbackError) {
            if (args.size() != 1 || !std::holds_alternative<std::uint64_t>(args[0]) ||
                std::get<std::uint64_t>(args[0]) != 0x2A) {
                callbackError = "unexpected decode callback argument";
                return false;
            }
            if (!reader.need(1) || reader.getUnsigned(1, false) != 0xA5) {
                callbackError = "unexpected callback marker";
                return false;
            }
            return true;
        }));

    std::string generatedError;
    REQUIRE(differential_callbacks::decode__differential_callbacks__DecodeCallbackPacket(
        wire, generated, consumed, &generatedCallbacks, &generatedError));
    REQUIRE(generatedError.empty());
    REQUIRE(consumed == reference.consumed);
    REQUIRE(generated.value == 0x2A);
}


TEST_CASE("generated C++ matches Reference Runtime for failed encode callback rollback") {
    std::string error;
    auto plan = makePlan(error);
    REQUIRE(plan);
    REQUIRE(error.empty());

    embx::runtime::CallbackRegistry referenceCallbacks;
    referenceCallbacks.add("differential_callbacks::on_encode_event", [](const std::string&, std::vector<std::uint8_t>& out,
                                                   std::size_t& pos, const embx::runtime::Environment&,
                                                   const std::vector<embx::runtime::Value>&, std::string& callbackError) {
        if (pos >= out.size()) out.resize(pos + 2);
        out[pos++] = 0xEE;
        out[pos++] = 0xFF;
        callbackError = "intentional encode callback failure";
        return false;
    });

    embx::encoder::Options options;
    options.callbacks = &referenceCallbacks;
    embx::value::Value::Object input;
    input["value"] = std::uint64_t{0x2A};
    const auto reference = embx::encoder::Engine(*plan, options).encode(
        "differential_callbacks::EncodeCallbackPacket", input);
    REQUIRE_FALSE(reference);
    REQUIRE(reference.error == "intentional encode callback failure");
    REQUIRE(reference.data.empty());

    differential_callbacks::EncodeCallbackPacket generated{};
    generated.value = 0x2A;
    embx_generated_detail::Callbacks generatedCallbacks;
    REQUIRE(generatedCallbacks.addEncode(
        "differential_callbacks::on_encode_event",
        [](const std::string&, embx_generated_detail::Writer& w,
           const std::nullptr_t&, const std::vector<embx_generated_detail::CallbackValue>&,
           std::string& callbackError) {
            if (!w.ensure(2)) { callbackError = "output overflow"; return false; }
            w.d[w.p++] = 0xEE;
            w.d[w.p++] = 0xFF;
            callbackError = "intentional encode callback failure";
            return false;
        }));

    std::vector<std::uint8_t> generatedBytes;
    std::string generatedError;
    REQUIRE_FALSE(differential_callbacks::encode__differential_callbacks__EncodeCallbackPacket(
        generated, generatedBytes, &generatedCallbacks, &generatedError));
    REQUIRE(generatedError == reference.error);
    REQUIRE(generatedBytes.empty());
}

TEST_CASE("generated C++ matches Reference Runtime for failed decode callback rollback") {
    std::string error;
    auto plan = makePlan(error);
    REQUIRE(plan);
    REQUIRE(error.empty());

    const std::vector<std::uint8_t> wire{0x2A, 0xA5};

    embx::runtime::CallbackRegistry referenceCallbacks;
    referenceCallbacks.add("differential_callbacks::on_decode_event", [](const std::string&, embx::runtime::Reader& reader,
                                                   const embx::runtime::Environment&,
                                                   const std::vector<embx::runtime::Value>&, std::string& callbackError) {
        REQUIRE(reader.remaining() == 1);
        REQUIRE(reader.uint(1) == 0xA5);
        callbackError = "intentional decode callback failure";
        return false;
    });

    embx::decoder::Engine decoder(*plan);
    decoder.callbacks(&referenceCallbacks);
    const auto reference = decoder.decode("differential_callbacks::DecodeCallbackPacket", wire);
    REQUIRE_FALSE(reference);
    REQUIRE(reference.error == "intentional decode callback failure");
    REQUIRE(reference.consumed == 0);

    differential_callbacks::DecodeCallbackPacket generated{};
    std::size_t consumed = 0;
    embx_generated_detail::Callbacks generatedCallbacks;
    REQUIRE(generatedCallbacks.addDecode(
        "differential_callbacks::on_decode_event",
        [](const std::string&, embx_generated_detail::Reader& reader,
           const std::nullptr_t&, const std::vector<embx_generated_detail::CallbackValue>&,
           std::string& callbackError) {
            if (!reader.need(1) || reader.getUnsigned(1, false) != 0xA5) {
                callbackError = "unexpected callback marker";
                return false;
            }
            callbackError = "intentional decode callback failure";
            return false;
        }));

    std::string generatedError;
    REQUIRE_FALSE(differential_callbacks::decode__differential_callbacks__DecodeCallbackPacket(
        wire, generated, consumed, &generatedCallbacks, &generatedError));
    REQUIRE(generatedError == reference.error);
    REQUIRE(consumed == reference.consumed);
    REQUIRE(generated.value == 0);
}
