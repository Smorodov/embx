#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "runtime/ExpressionEvaluator.h"
#include "core/Expr.h"
#include <cstdio>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace {
using Value = embx::value::Value;
using Object = Value::Object;

std::unique_ptr<embx::plan::Expr> lit(const char* text) {
    auto e = std::make_unique<embx::plan::Expr>();
    e->kind = embx::core::ExprKind::Literal;
    e->text = text;
    return e;
}

std::unique_ptr<embx::plan::Field> field(const char* name, const char* type) {
    auto f = std::make_unique<embx::plan::Field>();
    f->name = name;
    f->type.kind = embx::core::TypeKind::Primitive;
    f->type.name = type;
    return f;
}

void addStruct(embx::plan::Module& p, embx::plan::Struct s) {
    embx::test::addStruct(p, std::move(s));
}

bool rejectsSemantic(const std::string& source, const std::string& needle, int id) {
    const std::string path = "semantic_edge_cases_" + std::to_string(id) + ".embx";
    { std::ofstream f(path); f << source; }
    std::string err;
    auto m = embx::parser::parseFile(path, err);
    std::remove(path.c_str());
    if (!m || !err.empty()) return false;
    if (embx::semantic::analyze(*m, err)) return false;
    return err.find(needle) != std::string::npos;
}
}

TEST_CASE("semantic edge cases", "[semantic_edge_cases]") {
    using namespace embx;

    // Expression overflow remains an explicit runtime error at both signed and unsigned limits.
    runtime::Environment env;
    runtime::Value out;
    std::string err;
    auto plusOne = std::make_unique<core::Expr>();
    plusOne->kind = core::ExprKind::Binary;
    plusOne->op = "+";
    plusOne->left = std::make_unique<core::Expr>();
    plusOne->left->kind = core::ExprKind::Literal;
    plusOne->left->text = "9223372036854775807";
    plusOne->right = std::make_unique<core::Expr>();
    plusOne->right->kind = core::ExprKind::Literal;
    plusOne->right->text = "1";
    REQUIRE(!runtime::evaluate(plusOne.get(), env, out, err));
    REQUIRE(err == "integer overflow");

    env["U"] = std::numeric_limits<uint64_t>::max();
    auto unsignedOverflow = std::make_unique<core::Expr>();
    unsignedOverflow->kind = core::ExprKind::Binary;
    unsignedOverflow->op = "+";
    unsignedOverflow->left = std::make_unique<core::Expr>();
    unsignedOverflow->left->kind = core::ExprKind::Identifier;
    unsignedOverflow->left->text = "U";
    unsignedOverflow->right = std::make_unique<core::Expr>();
    unsignedOverflow->right->kind = core::ExprKind::Literal;
    unsignedOverflow->right->text = "1";
    REQUIRE(!runtime::evaluate(unsignedOverflow.get(), env, out, err));
    REQUIRE(err == "integer overflow");

    // Expression categories are rejected at semantic analysis time, rather than deferred to Plan.
    REQUIRE(rejectsSemantic("struct S { x: bytes[1.5]; }", "array length must be an integer expression", 1));
    REQUIRE(rejectsSemantic("struct S { x: u8; block b[\"x\"] { y: u8; } }", "block size must be an integer expression", 2));
    REQUIRE(rejectsSemantic("struct S { x: u8; at(2.5) { y: u8; } }", "at offset must be an integer expression", 3));
    REQUIRE(rejectsSemantic("struct S { align(1.5); }", "alignment must be an integer expression", 4));

    // Zero-length byte sequences are valid and round-trip without consuming data.
    plan::Module zeroPlan;
    plan::Struct zero; zero.name = "Zero";
    auto b = field("data", "bytes");
    b->type.dimensions.push_back(embx::core::Dimension::fixed(lit("0")));
    zero.members.push_back(std::move(b));
    auto tail = field("tail", "u8"); zero.members.push_back(std::move(tail));
    addStruct(zeroPlan, std::move(zero));

    decoder::Options dop; dop.requireFullInput = true;
    decoder::Engine dec(zeroPlan, dop);
    auto dr = dec.decode("Zero", {0x7F});
    REQUIRE(dr.success);
    REQUIRE(dr.consumed == 1);
    const auto& zo = std::get<Object>(dr.value.data);
    REQUIRE(std::get<Value::Bytes>(zo.at("data").data).empty());
    REQUIRE(std::get<uint64_t>(zo.at("tail").data) == 0x7F);

    Object zv;
    zv["data"] = Value::Bytes{};
    zv["tail"] = uint64_t(0x7F);
    encoder::Engine enc(zeroPlan);
    auto er = enc.encode("Zero", Value{zv});
    REQUIRE(er.success);
    REQUIRE(er.data == std::vector<uint8_t>{0x7F});

    // A bounded [*] field consumes exactly the remaining bytes of its block.
    plan::Module blockPlan;
    plan::Struct bs; bs.name = "Bounded";
    auto block = std::make_unique<plan::Block>();
    block->name = "payload";
    block->staticSize = 3;
    auto bytes = field("data", "bytes");
    bytes->type.dimensions.push_back(embx::core::Dimension::remaining());
    block->members.push_back(std::move(bytes));
    bs.members.push_back(std::move(block));
    addStruct(blockPlan, std::move(bs));
    decoder::Engine blockDec(blockPlan, dop);
    auto br = blockDec.decode("Bounded", {1, 2, 3});
    REQUIRE(br.success);
    REQUIRE(br.consumed == 3);
    const auto& bo = std::get<Object>(br.value.data);
    REQUIRE(std::get<Value::Bytes>(bo.at("data").data) == Value::Bytes({1, 2, 3}));

    // Allocation admission limits reject oversized byte sequences before allocation.
    decoder::Options limited = dop;
    limited.maxAllocationBytes = 2;
    plan::Module allocPlan;
    plan::Struct as; as.name = "Alloc";
    auto payload = field("payload", "bytes");
    payload->type.dimensions.push_back(embx::core::Dimension::fixed(lit("4")));
    as.members.push_back(std::move(payload));
    addStruct(allocPlan, std::move(as));
    decoder::Engine allocDec(allocPlan, limited);
    auto ar = allocDec.decode("Alloc", {1, 2, 3, 4});
    REQUIRE(!ar.success);
    REQUIRE(ar.error == "byte/string allocation exceeds decoder limit");

    // Root-relative at() cannot escape an active bounded block.
    plan::Module atPlan;
    plan::Struct ats; ats.name = "AtBounded";
    auto bounded = std::make_unique<plan::Block>();
    bounded->name = "payload";
    bounded->staticSize = 2;
    auto at = std::make_unique<plan::At>();
    at->offset = lit("2");
    at->members.push_back(field("x", "u8"));
    bounded->members.push_back(std::move(at));
    ats.members.push_back(std::move(bounded));
    addStruct(atPlan, std::move(ats));
    decoder::Engine atDec(atPlan, dop);
    auto atResult = atDec.decode("AtBounded", {0x10, 0x11});
    REQUIRE(!atResult.success);
    REQUIRE(atResult.error.find("integer read beyond reader limit") != std::string::npos);

    // Recursive structures are admitted by the type system but bounded at runtime by depth.
    plan::Module recPlan;
    plan::Struct node; node.name = "Node";
    const auto nodeId = test::addStruct(recPlan, std::move(node));
    auto next = field("next", "Node");
    next->type.kind = core::TypeKind::Named;
    next->type.reference.id = nodeId;
    test::addFieldSymbol(recPlan, *next);
    recPlan.structs.front().members.push_back(std::move(next));
    decoder::Options shallow; shallow.maxRecursionDepth = 1;
    decoder::Engine recDec(recPlan, shallow);
    auto rr = recDec.decode("Node", {0x00, 0x00});
    REQUIRE(!rr.success);
    REQUIRE(rr.error == "maximum recursion depth exceeded");
}

TEST_CASE("callback declarations require an explicit encode or decode direction") {
    const char* text = "callback on_packet();\n";
    std::ofstream f("callback_direction_test.embx"); f << text; f.close();
    std::string err;
    auto m = embx::parser::parseFile("callback_direction_test.embx", err);
    REQUIRE((m && err.empty()));
    REQUIRE_FALSE(embx::semantic::analyze(*m, err));
    REQUIRE(err.find("on_decode or on_encode") != std::string::npos);
    std::remove("callback_direction_test.embx");
}

TEST_CASE("callback uses must refer to a declared callback with matching direction") {
    const char* text = "struct Packet { callback on_encode_missing(); }\n";
    std::ofstream f("callback_declaration_test.embx"); f << text; f.close();
    std::string err;
    auto m = embx::parser::parseFile("callback_declaration_test.embx", err);
    REQUIRE((m && err.empty()));
    REQUIRE_FALSE(embx::semantic::analyze(*m, err));
    REQUIRE(err.find("callback is not declared") != std::string::npos);
    std::remove("callback_declaration_test.embx");
}
