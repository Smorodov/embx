#include <catch2/catch_test_macros.hpp>
#include "ir/IrBuilder.h"
#include "parser/ParserDriver.h"
#include "plan/PlanBuilder.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include <cstdio>
#include <fstream>
#include <memory>

TEST_CASE("Plan execution remains independent of AST and IR lifetime") {
    const char* path = "plan_execution_independence.embx";
    {
        std::ofstream f(path);
        f << R"(const N = 4;
struct Packet {
  count: u8;
  data: bytes[N];
  value: u16;
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

    // Deliberately destroy both compiler representations. The execution
    // contract must remain entirely owned by Plan.
    ir.reset();
    ast.reset();

    embx::value::Value::Object packet;
    packet["count"] = uint64_t(7);
    packet["data"] = std::vector<uint8_t>{1, 2, 3, 4};
    packet["value"] = uint64_t(0x1234);

    embx::encoder::Engine encoder(*plan);
    auto encoded = encoder.encode("Packet", packet);
    REQUIRE(encoded.success);
    REQUIRE(encoded.data.size() == 7);

    embx::decoder::Engine decoder(*plan);
    auto decoded = decoder.decode("Packet", encoded.data);
    REQUIRE(decoded.success);
    REQUIRE(decoded.consumed == encoded.data.size());

    const auto* object = std::get_if<embx::value::Value::Object>(&decoded.value.data);
    REQUIRE(object);
    REQUIRE(std::get<uint64_t>(object->at("count").data) == 7);
    REQUIRE(std::get<std::vector<uint8_t>>(object->at("data").data) == std::vector<uint8_t>({1,2,3,4}));
    REQUIRE(std::get<uint64_t>(object->at("value").data) == 0x1234);

    std::remove(path);
}

TEST_CASE("Plan construction is deterministic for identical source") {
    const char* path = "plan_determinism.embx";
    {
        std::ofstream f(path);
        f << R"(const A = 2;
const B = A + 3;
struct Packet {
  first: bytes[A];
  second: bytes[B];
})";
    }

    auto buildPlan = [&](std::unique_ptr<embx::plan::Module>& out) {
        std::string err;
        auto ast = embx::parser::parseFile(path, err);
        REQUIRE(ast);
        REQUIRE(err.empty());
        auto ir = embx::ir::lower(*ast, err);
        REQUIRE(ir);
        REQUIRE(err.empty());
        out = embx::plan::build(*ir, err);
        REQUIRE(out);
        REQUIRE(err.empty());
    };

    std::unique_ptr<embx::plan::Module> a;
    std::unique_ptr<embx::plan::Module> b;
    buildPlan(a);
    buildPlan(b);

    REQUIRE(a->symbolTable.findId("Packet") == b->symbolTable.findId("Packet"));
    REQUIRE(a->symbolTable.findId("A") == b->symbolTable.findId("A"));
    REQUIRE(a->symbolTable.findId("B") == b->symbolTable.findId("B"));
    REQUIRE(a->structs.size() == b->structs.size());
    REQUIRE(a->constants.size() == b->constants.size());
    REQUIRE(a->structs[0].name == b->structs[0].name);
    REQUIRE(a->structs[0].members.size() == b->structs[0].members.size());

    for (std::size_t i = 0; i < a->structs[0].members.size(); ++i) {
        const auto* af = dynamic_cast<const embx::plan::Field*>(a->structs[0].members[i].get());
        const auto* bf = dynamic_cast<const embx::plan::Field*>(b->structs[0].members[i].get());
        REQUIRE(af);
        REQUIRE(bf);
        REQUIRE(af->symbol == bf->symbol);
        REQUIRE(af->type.name == bf->type.name);
        REQUIRE(af->type.dimensions.size() == bf->type.dimensions.size());
        REQUIRE(af->staticLength == bf->staticLength);
        REQUIRE(af->type.dimensions[0].expression);
        REQUIRE(bf->type.dimensions[0].expression);
        REQUIRE(af->type.dimensions[0].expression->kind == bf->type.dimensions[0].expression->kind);
        REQUIRE(af->type.dimensions[0].expression->text == bf->type.dimensions[0].expression->text);
    }

    std::remove(path);
}
