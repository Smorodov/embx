#include <catch2/catch_test_macros.hpp>
#include "codegen/CppGenerator.h"
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
#include <filesystem>
#include <fstream>

static embx::codegen::Output generateCommon() {
    std::string error;
    auto ast = embx::parser::parseFile("../examples/common.embx", error);
    REQUIRE(ast);
    REQUIRE(error.empty());
    REQUIRE(embx::semantic::analyze(*ast, error));
    REQUIRE(error.empty());
    auto ir = embx::ir::lower(*ast, error);
    REQUIRE(ir);
    REQUIRE(error.empty());
    auto plan = embx::plan::build(*ir, error);
    REQUIRE(plan);
    REQUIRE(error.empty());
    embx::codegen::Output out;
    REQUIRE(embx::codegen::generateCpp(*plan, out, error));
    REQUIRE(error.empty());
    return out;
}

TEST_CASE("generated fixed scalar codec contains decode and encode") {
    const auto out = generateCommon();
    REQUIRE(out.header.find("decode__common__net__Header") != std::string::npos);
    REQUIRE(out.header.find("encode__common__net__Header") != std::string::npos);
    const bool hasBigEndianRead =
        out.source.find("getUnsigned(2,true)") != std::string::npos ||
        out.source.find("getUnsigned(2, true)") != std::string::npos;
    REQUIRE(hasBigEndianRead);
    const bool hasBigEndianWrite =
        out.source.find("putUnsigned(static_cast<std::uint64_t>(value.magic),2,true);") != std::string::npos ||
        out.source.find("putUnsigned(static_cast<std::uint64_t>(work.magic),2,true);") != std::string::npos ||
        out.source.find("putUnsigned(static_cast<std::uint64_t>(value.magic), 2, true);") != std::string::npos ||
        out.source.find("putUnsigned(static_cast<std::uint64_t>(work.magic), 2, true);") != std::string::npos;
    REQUIRE(hasBigEndianWrite);
    const bool hasByteRead =
        out.source.find("getUnsigned(1,true)") != std::string::npos ||
        out.source.find("getUnsigned(1, true)") != std::string::npos ||
        out.source.find("getUnsigned(1,false)") != std::string::npos ||
        out.source.find("getUnsigned(1, false)") != std::string::npos;
    REQUIRE(hasByteRead);
    const bool hasByteWrite =
        out.source.find("putUnsigned(static_cast<std::uint64_t>(value.kind),1,true);") != std::string::npos ||
        out.source.find("putUnsigned(static_cast<std::uint64_t>(value.kind), 1, true);") != std::string::npos ||
        out.source.find("putUnsigned(static_cast<std::uint64_t>(value.kind),1,false);") != std::string::npos ||
        out.source.find("putUnsigned(static_cast<std::uint64_t>(value.kind), 1, false);") != std::string::npos ||
        out.source.find("putUnsigned(static_cast<std::uint64_t>(work.kind),1,true);") != std::string::npos ||
        out.source.find("putUnsigned(static_cast<std::uint64_t>(work.kind), 1, true);") != std::string::npos ||
        out.source.find("putUnsigned(static_cast<std::uint64_t>(work.kind),1,false);") != std::string::npos ||
        out.source.find("putUnsigned(static_cast<std::uint64_t>(work.kind), 1, false);") != std::string::npos;
    REQUIRE(hasByteWrite);
}

TEST_CASE("generated encoder commits output transactionally") {
    const auto out = generateCommon();
    REQUIRE(out.source.find("std::vector<std::uint8_t> tmp;") != std::string::npos);
    REQUIRE(out.source.find("output.swap(tmp);") != std::string::npos);
}

TEST_CASE("generated codec keeps truncation as a checked decode failure") {
    const auto out = generateCommon();
    REQUIRE(out.header.find("input truncated") != std::string::npos);
    REQUIRE(out.source.find("if(!r.ok)") != std::string::npos);
}

TEST_CASE("generated C++ emits discriminator-driven variant codec") {
    const auto path = std::filesystem::temp_directory_path() / "embx_generated_variant_pass10.embx";
    std::ofstream out(path);
    out << R"(struct Packet {
  kind: u8;
  variant body by kind {
    1: u16;
    2: { value: u32; }
    default: u8;
  }
})";
    out.close();
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
    REQUIRE(embx::codegen::generateCpp(*plan, generated, error));
    REQUIRE(error.empty());
    REQUIRE(generated.header.find("#include <variant>") != std::string::npos);
    REQUIRE(generated.header.find("Packet_body") != std::string::npos);
    REQUIRE(generated.header.find("std::variant") != std::string::npos);
    REQUIRE(generated.source.find("variant payload does not match discriminator") != std::string::npos);
    REQUIRE(generated.source.find("variant payload does not match default discriminator") != std::string::npos);

    // A variant without a default must emit the explicit no-match failure path.
    const auto noDefaultPath = std::filesystem::temp_directory_path() / "embx_generated_variant_pass10_no_default.embx";
    std::ofstream noDefaultOut(noDefaultPath);
    noDefaultOut << R"(struct PacketNoDefault {
  kind: u8;
  variant body by kind {
    1: u16;
    2: { value: u32; }
  }
})";
    noDefaultOut.close();
    std::string noDefaultError;
    auto noDefaultAst = embx::parser::parseFile(noDefaultPath.string(), noDefaultError);
    REQUIRE(noDefaultAst);
    REQUIRE(noDefaultError.empty());
    auto noDefaultIr = embx::ir::lower(*noDefaultAst, noDefaultError);
    REQUIRE(noDefaultIr);
    REQUIRE(noDefaultError.empty());
    auto noDefaultPlan = embx::plan::build(*noDefaultIr, noDefaultError);
    REQUIRE(noDefaultPlan);
    REQUIRE(noDefaultError.empty());
    embx::codegen::Output noDefaultGenerated;
    REQUIRE(embx::codegen::generateCpp(*noDefaultPlan, noDefaultGenerated, noDefaultError));
    REQUIRE(noDefaultError.empty());
    REQUIRE(noDefaultGenerated.source.find("no variant case matched") != std::string::npos);

    std::error_code ec;
    std::filesystem::remove(path, ec);
    std::filesystem::remove(noDefaultPath, ec);
}


TEST_CASE("generated C++ emits virtual fields and aliases") {
    const auto path = std::filesystem::temp_directory_path() / "embx_generated_virtual_alias_pass11.embx";
    std::ofstream out(path);
    out << R"(struct Packet {
  value: u8;
  let doubled = value * 2;
  alias value2 = value;
})";
    out.close();
    std::string error;
    auto ast = embx::parser::parseFile(path.string(), error);
    REQUIRE(ast); REQUIRE(error.empty());
    auto ir = embx::ir::lower(*ast, error);
    REQUIRE(ir); REQUIRE(error.empty());
    auto plan = embx::plan::build(*ir, error);
    REQUIRE(plan); REQUIRE(error.empty());
    embx::codegen::Output generated;
    REQUIRE(embx::codegen::generateCpp(*plan, generated, error));
    REQUIRE(error.empty());
    REQUIRE(generated.header.find("std::uint64_t doubled{}") != std::string::npos);
    REQUIRE(generated.header.find("auto& value2()") != std::string::npos);
    REQUIRE(generated.source.find("out.doubled=") != std::string::npos);
    std::filesystem::remove(path);
}

TEST_CASE("generated C++ emits scale transform in logical type and codec") {
    const auto path = std::filesystem::temp_directory_path() / "embx_generated_transform_pass11.embx";
    std::ofstream out(path);
    out << R"(struct Packet {
  raw: i16 transform scale(0.1);
})";
    out.close();
    std::string error;
    auto ast = embx::parser::parseFile(path.string(), error);
    REQUIRE(ast); REQUIRE(error.empty());
    auto ir = embx::ir::lower(*ast, error);
    REQUIRE(ir); REQUIRE(error.empty());
    auto plan = embx::plan::build(*ir, error);
    REQUIRE(plan); REQUIRE(error.empty());
    embx::codegen::Output generated;
    REQUIRE(embx::codegen::generateCpp(*plan, generated, error));
    REQUIRE(error.empty());
    REQUIRE(generated.header.find("double raw{}") != std::string::npos);
    REQUIRE(generated.source.find("/ 0.100000") != std::string::npos);
    REQUIRE(generated.source.find("* 0.100000") != std::string::npos);
    std::filesystem::remove(path);
}

TEST_CASE("generated C++ emits terminated byte sequence codec") {
    const auto path = std::filesystem::temp_directory_path() / "embx_generated_terminated_sequence.embx";
    std::ofstream out(path);
    out << R"(struct Record {
  payload: bytes until 0xDE 0xAD max 8;
  tail: u8;
})";
    out.close();

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
    REQUIRE(embx::codegen::generateCpp(*plan, generated, error));
    REQUIRE(error.empty());
    REQUIRE(generated.header.find("getTerminatedBytes") != std::string::npos);
    REQUIRE(generated.header.find("putTerminatedBytes") != std::string::npos);
    REQUIRE(generated.source.find("getTerminatedBytes") != std::string::npos);
    REQUIRE(generated.source.find("putTerminatedBytes") != std::string::npos);
    REQUIRE(generated.source.find("8ULL") != std::string::npos);

    std::error_code ec;
    std::filesystem::remove(path, ec);
}
