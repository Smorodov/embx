#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "compiler/Compiler.h"
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"
#include "runtime/Runtime.h"

namespace {
std::shared_ptr<embx::plan::Module> compileLesson(std::string& error) {
    const auto path = std::filesystem::path("../course/10_callbacks_transforms/callbacks_transforms.embx");
    auto compilation = embx::compiler::compileFile(path.string(), error);
    if (!compilation) return {};
    return std::shared_ptr<embx::plan::Module>(std::move(compilation->plan));
}
}

TEST_CASE("lesson 10 source compiles into an executable plan") {
    std::string error;
    auto plan = compileLesson(error);
    REQUIRE(plan);
    REQUIRE(error.empty());
    REQUIRE(plan->structIndexBySymbol.size() >= 3);
}

TEST_CASE("decode callback receives the decoded field value") {
    std::string error;
    auto plan = compileLesson(error);
    REQUIRE(plan);

    embx::runtime::CallbackRegistry registry;
    bool invoked = false;
    registry.add("on_decode_event", [&](const std::string&, embx::runtime::Reader&, const embx::runtime::Environment&, const std::vector<embx::runtime::Value>& args, std::string& callbackError) {
        if (args.size() != 1 || !std::holds_alternative<uint64_t>(args[0]) || std::get<uint64_t>(args[0]) != 0x2A) {
            callbackError = "unexpected decode callback argument";
            return false;
        }
        invoked = true;
        return true;
    });

    embx::decoder::Engine decoder(*plan);
    decoder.callbacks(&registry);
    auto result = decoder.decode("DecodeEvent", {0x2A});

    INFO("decode error: " << result.error);
    REQUIRE(result);
    REQUIRE(invoked);
    REQUIRE(std::get<uint64_t>(result.value.at("value").data) == 0x2A);
}

TEST_CASE("encode callback receives the logical field value") {
    std::string error;
    auto plan = compileLesson(error);
    REQUIRE(plan);

    embx::runtime::CallbackRegistry registry;
    bool invoked = false;
    registry.add("on_encode_event", [&](const std::string&, std::vector<uint8_t>&, size_t&, const embx::runtime::Environment&, const std::vector<embx::runtime::Value>& args, std::string& callbackError) {
        if (args.size() != 1 || !std::holds_alternative<uint64_t>(args[0]) || std::get<uint64_t>(args[0]) != 0x2A) {
            callbackError = "unexpected encode callback argument";
            return false;
        }
        invoked = true;
        return true;
    });

    embx::encoder::Options options;
    options.callbacks = &registry;
    embx::encoder::Engine encoder(*plan, options);
    embx::value::Value::Object input;
    input["value"] = uint64_t(0x2A);
    auto result = encoder.encode("EncodeEvent", input);

    INFO("encode error: " << result.error);
    REQUIRE(result);
    REQUIRE(invoked);
    REQUIRE(result.data == std::vector<uint8_t>{0x2A});
}

TEST_CASE("scale transforms wire and logical values without changing layout") {
    std::string error;
    auto plan = compileLesson(error);
    REQUIRE(plan);

    embx::decoder::Engine decoder(*plan);
    auto decoded = decoder.decode("Measurement", {0x00, 0xFD});
    REQUIRE(decoded);
    REQUIRE(std::get<double>(decoded.value.at("temperature").data) == Catch::Approx(25.3));

    embx::value::Value::Object input;
    input["temperature"] = 25.3;
    embx::encoder::Engine encoder(*plan);
    auto encoded = encoder.encode("Measurement", input);
    REQUIRE(encoded);
    REQUIRE(encoded.data == std::vector<uint8_t>{0x00, 0xFD});

    const auto id = plan->symbolTable.findId("Measurement");
    REQUIRE(id != embx::core::InvalidSymbolId);
    const auto& s = plan->structs[plan->structIndexBySymbol.at(id)];
    REQUIRE(s.staticSize == 2);
}
