#include <catch2/catch_test_macros.hpp>
#include "codegen/CppGenerator.h"
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
#include <string>

static embx::codegen::Output generateNested() {
    std::string error;
    auto ast = embx::parser::parseFile("../examples/nested_types.embx", error);
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

TEST_CASE("generator emits enum, nested structs and aliases") {
    const auto out = generateNested();
    REQUIRE(out.header.find("enum class Kind : std::uint8_t") != std::string::npos);
    REQUIRE(out.header.find("struct Point {") != std::string::npos);
    REQUIRE(out.header.find("using PointAlias = ::demo::nested::Point;") != std::string::npos);
    REQUIRE(out.header.find("::demo::nested::Point head{};") != std::string::npos);
    REQUIRE(out.header.find("std::array<::demo::nested::Point") != std::string::npos);
    REQUIRE(out.header.find(" points{};") != std::string::npos);
}

TEST_CASE("generated nested codec uses shared struct and enum bodies") {
    const auto out = generateNested();
    REQUIRE(out.source.find("decode_body_demo__nested__Point") != std::string::npos);
    REQUIRE(out.source.find("encode_body_demo__nested__Point") != std::string::npos);
    const bool hasEnumRead =
        out.source.find("static_cast<::demo::nested::Kind>(r.getUnsigned(1,true))") != std::string::npos ||
        out.source.find("static_cast<::demo::nested::Kind>(r.getUnsigned(1, true))") != std::string::npos;
    REQUIRE(hasEnumRead);
    REQUIRE(out.source.find("decode_body_demo__nested__Point(r,out.head,error)") != std::string::npos);
    REQUIRE(out.source.find("decode_body_demo__nested__Point") != std::string::npos);
    const bool hasPointsIndex =
        out.source.find("out.points[i0]") != std::string::npos ||
        out.source.find("out.points[i]") != std::string::npos;
    REQUIRE(hasPointsIndex);
    const bool hasNestedEncodeCall =
        out.source.find("encode_body_demo__nested__Point(value.head,output,error)") != std::string::npos ||
        out.source.find("encode_body_demo__nested__Point(work.head,output,error)") != std::string::npos;
    REQUIRE(hasNestedEncodeCall);
    const bool hasPointsEncodeLoop =
        out.source.find("for(std::size_t i0=0;i0<work.points.size();++i0)") != std::string::npos ||
        out.source.find("for(std::size_t i0=0;i0<value.points.size();++i0)") != std::string::npos ||
        out.source.find("for(std::size_t i0=0;i0<2ULL;++i0)") != std::string::npos ||
        out.source.find("for(const auto& item:value.points)") != std::string::npos ||
        out.source.find("for(const auto& item:work.points)") != std::string::npos;
    REQUIRE(hasPointsEncodeLoop);
}
