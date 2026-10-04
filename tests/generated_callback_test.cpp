#include <catch2/catch_test_macros.hpp>
#include "codegen/CppGenerator.h"
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
#include <filesystem>
#include <fstream>

static std::unique_ptr<embx::plan::Module> makeCallbackPlan(const std::string& name, std::string& error) {
    const auto path = std::filesystem::temp_directory_path() / name;
    std::ofstream f(path);
    f << R"(callback on_decode_value(ctx);
callback on_encode_value(ctx);
struct Packet {
  value: u8;
  callback on_decode_value(value);
  callback on_encode_value(value);
})";
    f.close();
    auto ast = embx::parser::parseFile(path.string(), error);
    REQUIRE(ast);
    REQUIRE(error.empty());
    auto ir = embx::ir::lower(*ast, error);
    REQUIRE(ir);
    REQUIRE(error.empty());
    auto plan = embx::plan::build(*ir, error);
    REQUIRE(plan);
    REQUIRE(error.empty());
    return plan;
}

TEST_CASE("generated C++ closes the callback boundary for both codec directions") {
    std::string error;
    auto plan = makeCallbackPlan("embx_generated_callback_pass14.embx", error);

    embx::codegen::Output generated;
    INFO("generateCpp error: " << error);
    REQUIRE(embx::codegen::generateCpp(*plan, generated, error));
    REQUIRE(error.empty());

    REQUIRE(generated.header.find("using CallbackValue = std::variant") != std::string::npos);
    REQUIRE(generated.header.find("struct Callbacks") != std::string::npos);
    REQUIRE(generated.header.find("addDecode") != std::string::npos);
    REQUIRE(generated.header.find("addEncode") != std::string::npos);
    REQUIRE(generated.header.find("const embx_generated_detail::Callbacks* callbacks=nullptr") != std::string::npos);

    REQUIRE(generated.source.find("callDecode(\"on_decode_value\"") != std::string::npos);
    REQUIRE(generated.source.find("callEncode(\"on_encode_value\"") != std::string::npos);
    REQUIRE(generated.source.find("static_cast<std::uint64_t>(out.value)") != std::string::npos);
    REQUIRE(generated.source.find("static_cast<std::uint64_t>(work.value)") != std::string::npos);

    // The opposite-direction callback must not be emitted into either codec path.
    const auto decodeBody = generated.source.find("static bool decode_body_Packet");
    const auto encodeBody = generated.source.find("static bool encode_body_Packet");
    REQUIRE(decodeBody != std::string::npos);
    REQUIRE(encodeBody != std::string::npos);
    const auto decodeEnd = generated.source.find("static bool encode_body_Packet", decodeBody);
    const auto encodeEnd = generated.source.find("bool decode__", encodeBody);
    REQUIRE(decodeEnd != std::string::npos);
    REQUIRE(encodeEnd != std::string::npos);

    const auto decodeText = generated.source.substr(decodeBody, decodeEnd - decodeBody);
    const auto encodeText = generated.source.substr(encodeBody, encodeEnd - encodeBody);
    REQUIRE(decodeText.find("callDecode(\"on_decode_value\"") != std::string::npos);
    REQUIRE(decodeText.find("callEncode(\"on_encode_value\"") == std::string::npos);
    REQUIRE(encodeText.find("callEncode(\"on_encode_value\"") != std::string::npos);
    REQUIRE(encodeText.find("callDecode(\"on_decode_value\"") == std::string::npos);
}

TEST_CASE("generated C++ skips opposite-direction callbacks inside at") {
    const auto path = std::filesystem::temp_directory_path() / "embx_generated_callback_at_pass14.embx";
    std::ofstream f(path);
    f << R"(callback on_decode_value(ctx);
callback on_encode_value(ctx);
struct Packet {
  value: u8;
  at(8) {
    callback on_decode_value(value);
    callback on_encode_value(value);
  }
})";
    f.close();

    std::string error;
    auto ast = embx::parser::parseFile(path.string(), error);
    REQUIRE(ast);
    REQUIRE(error.empty());
    auto ir = embx::ir::lower(*ast, error);
    REQUIRE(ir);
    REQUIRE(error.empty());
    auto plan = embx::plan::build(*ir, error);
    REQUIRE(plan);
    REQUIRE(error.empty());

    embx::codegen::Output generated;
    INFO("generateCpp error: " << error);
    REQUIRE(embx::codegen::generateCpp(*plan, generated, error));
    REQUIRE(error.empty());
    REQUIRE(generated.source.find("callDecode(\"on_decode_value\"") != std::string::npos);
    REQUIRE(generated.source.find("callEncode(\"on_encode_value\"") != std::string::npos);
}
