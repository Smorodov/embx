#include <catch2/catch_test_macros.hpp>
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
#include "reflection/Reflection.h"
#include <fstream>
#include <filesystem>

TEST_CASE("runtime parameters form executable module metadata") {
    const auto path = std::filesystem::temp_directory_path() / "embx_runtime_parameters.embx";
    std::ofstream out(path);
    out << "param count: u32;\nparam delta: u32;\nstruct S { header: u8; block payload[count] { data: bytes[*]; } at($next + delta) { marker: u8; } }\n";
    out.close(); std::string err;
    auto ast = embx::parser::parseFile(path.string(), err); REQUIRE(ast); REQUIRE(err.empty()); REQUIRE(ast->parameters.size() == 2);
    REQUIRE(embx::semantic::analyze(*ast, err)); REQUIRE(err.empty());
    auto ir = embx::ir::lower(*ast, err); REQUIRE(ir); REQUIRE(err.empty()); REQUIRE(ir->parameters.size() == 2);
    REQUIRE(ir->symbolTable.find(ir->parameters[0].symbol)->kind == embx::core::SymbolKind::Parameter);
    auto plan = embx::plan::build(*ir, err); REQUIRE(plan); REQUIRE(err.empty()); REQUIRE(plan->parameters.size() == 2);
    REQUIRE(plan->parameterIndexBySymbol.count(plan->parameters[0].symbol) == 1);
    const auto refl = embx::reflection::inspect(*plan); REQUIRE(refl.parameters.size() == 2);
    REQUIRE(refl.parameters[0].name == "count"); REQUIRE(refl.parameters[0].type == "u32");
    std::filesystem::remove(path);
}

TEST_CASE("runtime parameter declarations reject non scalar types") {
    const auto path = std::filesystem::temp_directory_path() / "embx_runtime_parameter_bad.embx";
    std::ofstream out(path); out << "param bad: bytes[4];\n"; out.close(); std::string err;
    auto ast = embx::parser::parseFile(path.string(), err); REQUIRE(ast); REQUIRE_FALSE(embx::semantic::analyze(*ast, err));
    REQUIRE(err.find("scalar primitive") != std::string::npos); std::filesystem::remove(path);
}

#include "encoder/Encoder.h"
#include "decoder/Decoder.h"

TEST_CASE("runtime parameters drive dynamic codec execution") {
    const auto path = std::filesystem::temp_directory_path() / "embx_runtime_parameter_codec.embx";
    std::ofstream out(path);
    out << "param count: u32;\nparam delta: u32;\nstruct S { header: u8; block payload[count] { data: bytes[*]; } at($next + delta) { marker: u8; } }\n";
    out.close();
    std::string err; auto ast = embx::parser::parseFile(path.string(), err); REQUIRE(ast); REQUIRE(embx::semantic::analyze(*ast, err));
    auto ir = embx::ir::lower(*ast, err); REQUIRE(ir); auto plan = embx::plan::build(*ir, err); REQUIRE(plan);
    const auto count = plan->symbolTable.findId("count"); const auto delta = plan->symbolTable.findId("delta");
    REQUIRE(count != embx::core::InvalidSymbolId); REQUIRE(delta != embx::core::InvalidSymbolId);
    embx::encoder::Options eo; eo.parameters[count] = uint64_t(2); eo.parameters[delta] = uint64_t(1);
    embx::value::Value::Object object; object["header"] = uint64_t(7); object["data"] = embx::value::Value(embx::value::Value::Bytes{0xAA,0xBB}); object["marker"] = uint64_t(9);
    embx::encoder::Engine encoder(*plan, eo); auto encoded = encoder.encode("S", embx::value::Value(object));
    REQUIRE(encoded); REQUIRE(encoded.data.size() == 5); REQUIRE(encoded.data[0] == 7); REQUIRE(encoded.data[3] == 0); REQUIRE(encoded.data[4] == 9);
    embx::decoder::Options dopt; dopt.parameters[count] = uint64_t(2); dopt.parameters[delta] = uint64_t(1); dopt.requireFullInput = false;
    embx::decoder::Engine decoder(*plan, dopt); auto decoded = decoder.decode("S", encoded.data);
    REQUIRE(decoded); REQUIRE(decoded.consumed == 3);
    auto* header = decoded.value.get("header");
    REQUIRE(header);
    REQUIRE(std::get<uint64_t>(header->data) == 7);
    auto* marker = decoded.value.get("marker");
    REQUIRE(marker);
    REQUIRE(std::get<uint64_t>(marker->data) == 9);
    std::filesystem::remove(path);
}


TEST_CASE("encoder rejects missing runtime parameters") {
    const auto path = std::filesystem::temp_directory_path() / "embx_runtime_parameter_missing.embx";
    std::ofstream out(path);
    out << "param enabled: u8;\nstruct S { value: u8; if (enabled == 1) { extra: u8; } }\n";
    out.close();
    std::string err;
    auto ast = embx::parser::parseFile(path.string(), err); REQUIRE(ast); REQUIRE(err.empty());
    REQUIRE(embx::semantic::analyze(*ast, err)); REQUIRE(err.empty());
    auto ir = embx::ir::lower(*ast, err); REQUIRE(ir); REQUIRE(err.empty());
    auto plan = embx::plan::build(*ir, err); REQUIRE(plan); REQUIRE(err.empty());

    embx::value::Value::Object object;
    object["value"] = uint64_t(7);
    embx::encoder::Engine encoder(*plan);
    auto encoded = encoder.encode("S", embx::value::Value(object));
    REQUIRE_FALSE(encoded.success);
    REQUIRE(encoded.error == "missing parameter: enabled");
    std::filesystem::remove(path);
}
