#include <catch2/catch_test_macros.hpp>
#include "ir/IrBuilder.h"
#include "parser/ParserDriver.h"
#include "plan/PlanBuilder.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "runtime/ExpressionEvaluator.h"
#include <cstdio>
#include <fstream>
#include <memory>

TEST_CASE("runtime dynamic expressions use SymbolId rather than expression spelling") {
    const char* path = "runtime_symbol_expression.embx";
    {
        std::ofstream f(path);
        f << R"(struct Packet {
  count: u8;
  data: bytes[count];
})";
    }

    std::string err;
    auto ast = embx::parser::parseFile(path, err);
    REQUIRE(ast);
    REQUIRE(err.empty());
    auto ir = embx::ir::lower(*ast, err);
    REQUIRE(ir);
    REQUIRE(err.empty());
    auto plan = embx::plan::build(*ir, err);
    REQUIRE(plan);
    REQUIRE(err.empty());

    const auto packetId = plan->symbolTable.findId("Packet");
    REQUIRE(packetId != embx::core::InvalidSymbolId);
    const auto countId = plan->symbolTable.findId("count");
    REQUIRE(countId == embx::core::InvalidSymbolId); // fields are scoped

    const auto structIt = plan->structIndexBySymbol.find(packetId);
    REQUIRE(structIt != plan->structIndexBySymbol.end());
    auto& packet = plan->structs[structIt->second];
    REQUIRE(packet.members.size() == 2);

    auto* data = dynamic_cast<embx::plan::Field*>(packet.members[1].get());
    REQUIRE(data);
    REQUIRE(data->type.dimensions.size() == 1);
    REQUIRE(data->type.dimensions[0].expression);
    REQUIRE(data->type.dimensions[0].expression->reference.valid());

    const auto countSymbol = data->type.dimensions[0].expression->reference.id;
    REQUIRE(countSymbol != embx::core::InvalidSymbolId);
    REQUIRE(data->type.dimensions[0].expression->text == "count");

    // Corrupt only the source spelling retained for diagnostics. Execution
    // must continue to use the canonical SymbolId reference.
    data->type.dimensions[0].expression->text = "name_that_does_not_exist";

    embx::value::Value::Object packetValue;
    packetValue["count"] = uint64_t(3);
    packetValue["data"] = std::vector<uint8_t>{9, 8, 7};

    embx::encoder::Engine encoder(*plan);
    const auto encoded = encoder.encode("Packet", packetValue);
    REQUIRE(encoded.success);
    REQUIRE(encoded.data == std::vector<uint8_t>({3, 9, 8, 7}));

    embx::decoder::Engine decoder(*plan);
    const auto decoded = decoder.decode("Packet", encoded.data);
    REQUIRE(decoded.success);
    REQUIRE(decoded.consumed == encoded.data.size());

    std::remove(path);
}

TEST_CASE("runtime expression evaluator rejects unresolved SymbolId without semantic name repair") {
    embx::core::Expr expr;
    expr.kind = embx::core::ExprKind::Identifier;
    expr.text = "count";
    expr.reference.id = 42;

    embx::runtime::SymbolEnvironment env;
    env.emplace(7, uint64_t(3));

    embx::runtime::Value out;
    std::string err;
    REQUIRE_FALSE(embx::runtime::evaluate(&expr, env, out, err));
    REQUIRE(err == "unknown symbol: count");
}
