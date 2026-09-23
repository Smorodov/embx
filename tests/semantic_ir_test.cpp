#include <catch2/catch_test_macros.hpp>
#include "ir/IrBuilder.h"
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include <cstdio>
#include <fstream>
#include <iostream>

namespace {
void fail(const char* msg) {
    FAIL(msg);
}
}

TEST_CASE("semantic ir", "[semantic_ir]") {
    const char* path = "semantic_ir_test.embx";
    std::ofstream g(path);
    g << R"(type Payload = bytes[*];
const N = 2 + 2 * 2;
enum Kind: u8 { A = 1, B = 2 }
callback on_decode_cb(arg);
struct Packet {
 magic: bytes[4] = "PKT0";
 kind: u8;
 block payload[N] {
   variant body by kind {
     1: { id: u32; value: u16; }
     2: bytes[*];
     default: Payload;
   }
 }
 align(4);
 at(64) { index: u32; }
})";
    g.close();

    std::string err;
    auto a = embx::parser::parseFile(path, err);
    if (!a || !err.empty()) {
        std::remove(path);
        fail((std::string("parse failed: ") + err).c_str());
    }
    if (!embx::semantic::analyze(*a, err)) {
        std::remove(path);
        fail((std::string("semantic failed: ") + err).c_str());
    }

    auto ir = embx::ir::lower(*a, err);
    if (!ir || !err.empty()) {
        std::remove(path);
        fail((std::string("IR lowering failed: ") + err).c_str());
    }

    const auto checkSymbolKind = [&](const char* name, embx::core::SymbolKind expected, const char* message) {
        const auto id = ir->symbolTable.findId(name);
        const auto* symbol = ir->symbolTable.find(id);
        if (!symbol || symbol->kind != expected) fail(message);
    };
    checkSymbolKind("Payload", embx::core::SymbolKind::TypeAlias, "Payload symbol kind");
    checkSymbolKind("N", embx::core::SymbolKind::Constant, "N symbol kind");
    checkSymbolKind("Kind", embx::core::SymbolKind::Enum, "Kind symbol kind");
    checkSymbolKind("on_decode_cb", embx::core::SymbolKind::Callback, "cb symbol kind");
    checkSymbolKind("Packet", embx::core::SymbolKind::Struct, "Packet symbol kind");
    if (ir->structs.size() != 1) fail("expected one IR struct");

    const auto& members = ir->structs[0]->members;
    // Packet contains: field, field, block, align, at.
    if (members.size() != 5) fail("expected five Packet members");
    if (members[0]->kind != embx::ir::MemberKind::Field) fail("member 0 kind");
    if (members[1]->kind != embx::ir::MemberKind::Field) fail("member 1 kind");
    if (members[2]->kind != embx::ir::MemberKind::Block) fail("member 2 kind");
    if (members[3]->kind != embx::ir::MemberKind::Align) fail("member 3 kind");
    if (members[4]->kind != embx::ir::MemberKind::At) fail("member 4 kind");

    auto* f0 = dynamic_cast<embx::ir::Field*>(members[0].get());
    if (!f0 || f0->name != "magic" || f0->type.name != "bytes" || f0->type.dimensions.size() != 1 || f0->type.dimensions[0].kind != embx::core::Dimension::Kind::Fixed || f0->assertion != "\"PKT0\"")
        fail("magic field lowering");

    auto* b = dynamic_cast<embx::ir::Block*>(members[2].get());
    if (!b || b->name != "payload" || !b->size) fail("block lowering");
    if (b->members.size() != 1 || b->members[0]->kind != embx::ir::MemberKind::Variant) fail("block variant lowering");

    auto* v = dynamic_cast<embx::ir::Variant*>(b->members[0].get());
    if (!v || v->name != "body" || !v->discriminator || v->cases.size() != 2 || !v->hasDefault)
        fail("variant lowering");
    if (v->cases[0].members.size() != 2) fail("variant case member lowering");
    if (!v->cases[1].hasType || v->cases[1].type.name != "bytes" || v->cases[1].type.dimensions.size() != 1 || v->cases[1].type.dimensions[0].kind != embx::core::Dimension::Kind::Remaining)
        fail("dynamic bytes case lowering");
    if (v->defaultType.name != "Payload") fail("variant default type lowering");

    auto* al = dynamic_cast<embx::ir::Align*>(members[3].get());
    if (!al || !al->alignment) fail("align lowering");
    auto* at = dynamic_cast<embx::ir::At*>(members[4].get());
    if (!at || !at->offset || at->members.size() != 1 || at->members[0]->kind != embx::ir::MemberKind::Field)
        fail("at lowering");

    std::remove(path);
    std::cout << "Semantic IR tests passed\n";
}
