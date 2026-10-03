#include <catch2/catch_test_macros.hpp>
#include "ir/IrBuilder.h"
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "core/Semantic.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <type_traits>
#include "core/Type.h"

TEST_CASE("ir structural", "[ir_structural]") {
    const char* path = "ir_structural_test.embx";
    std::ofstream f(path);
    f << R"(@ endian little
 type Data = bytes[*];
 const N = 4;
 enum Kind: u8 { A = 1, B = 2 }
 callback on_decode(buf);
 struct First { a: u8; }
 computed M = N + 1;
 type Word = u16;
 struct S big {
   a: u16 little [N] = 7;
   bits { lo: u8 (3); hi: u8 (5); }
   variant v by a { 1: u8; 2: { x: u32; } default: Data; }
   block payload[N] { y: u8; }
   at(64) { z: u16; }
   align(4);
   callback on_decode(a, N);
 }
 enum Tail: u16 { X = 9 }
 )";
    f.close();

    std::string err;
    auto ast = embx::parser::parseFile(path, err);
    REQUIRE((ast && err.empty()));
    REQUIRE(embx::semantic::analyze(*ast, err));
    auto ir = embx::ir::lower(*ast, err);
    REQUIRE((ir && err.empty()));

    REQUIRE(ir->endian == embx::core::Endian::Little);

    // Source order must be preserved, while each IR reference indexes the
    // corresponding typed storage vector.
    REQUIRE(ir->order.size() == 9);
    REQUIRE((ir->order[0].kind == embx::ir::TopLevelRef::Kind::Alias && ir->order[0].index == 0));    // Data
    REQUIRE((ir->order[1].kind == embx::ir::TopLevelRef::Kind::Const && ir->order[1].index == 0));    // N
    REQUIRE((ir->order[2].kind == embx::ir::TopLevelRef::Kind::Enum && ir->order[2].index == 0));     // Kind
    REQUIRE((ir->order[3].kind == embx::ir::TopLevelRef::Kind::Callback && ir->order[3].index == 0)); // decode
    REQUIRE((ir->order[4].kind == embx::ir::TopLevelRef::Kind::Struct && ir->order[4].index == 0));   // First
    REQUIRE((ir->order[5].kind == embx::ir::TopLevelRef::Kind::Const && ir->order[5].index == 1));    // M
    REQUIRE((ir->order[6].kind == embx::ir::TopLevelRef::Kind::Alias && ir->order[6].index == 1));   // Word
    REQUIRE((ir->order[7].kind == embx::ir::TopLevelRef::Kind::Struct && ir->order[7].index == 1));  // S
    REQUIRE((ir->order[8].kind == embx::ir::TopLevelRef::Kind::Enum && ir->order[8].index == 1));     // Tail

    REQUIRE(ir->aliases.size() == 2);
    REQUIRE((ir->aliases[0].name == "Data" && ir->aliases[0].target.name == "bytes"));
    REQUIRE(ir->aliases[0].target.dimensions.size() == 1);
    REQUIRE(ir->aliases[0].target.dimensions[0].kind == embx::core::Dimension::Kind::Remaining);
    static_assert(std::is_same<embx::ir::Type, embx::core::Type>::value, "IR Type must be canonical core Type");
    REQUIRE((ir->aliases[1].name == "Word" && ir->aliases[1].target.name == "u16"));
    REQUIRE((ir->enums.size() == 2 && ir->enums[0].hasUnderlying && ir->enums[1].name == "Tail"));
    REQUIRE((ir->constants.size() == 2 && !ir->constants[0].computed && ir->constants[1].computed));
    REQUIRE((ir->callbacks.size() == 1 && ir->callbacks[0].hasParameter && ir->callbacks[0].parameter == "buf" && ir->callbacks[0].direction == embx::core::CallbackDirection::Decode));
    REQUIRE((ir->structs.size() == 2 && ir->structs[0]->name == "First" && ir->structs[1]->name == "S"));

    const auto& ms = ir->structs[1]->members;
    REQUIRE(ms.size() == 7);
    auto* field = dynamic_cast<embx::ir::Field*>(ms[0].get());
    REQUIRE((field && field->assertion == "7"));
    REQUIRE(field->endian == embx::core::Endian::Little);
    // Field-level [N] is normalized into the canonical Type shape.
    REQUIRE(field->type.dimensions.size() == 1);
    REQUIRE(field->type.dimensions[0].kind == embx::core::Dimension::Kind::Dynamic);
    REQUIRE(field->type.dimensions[0].expression != nullptr);
    // There is intentionally no separate IR field-length representation: the
    // source-level [N] modifier is normalized into Type::dimensions.

    auto* bits = dynamic_cast<embx::ir::Bits*>(ms[1].get());
    REQUIRE((bits && bits->fields.size() == 2 && bits->totalBits == 8));
    auto* v = dynamic_cast<embx::ir::Variant*>(ms[2].get());
    REQUIRE((v && v->cases.size() == 2 && v->hasDefault && v->defaultType.name == "Data"));
    REQUIRE((v->cases[0].hasType && v->cases[0].type.name == "u8"));
    REQUIRE((!v->cases[1].hasType && v->cases[1].members.size() == 1));
    auto* block = dynamic_cast<embx::ir::Block*>(ms[3].get());
    REQUIRE((block && block->size && block->members.size() == 1));
    auto* at = dynamic_cast<embx::ir::At*>(ms[4].get());
    REQUIRE((at && at->offset && at->members.size() == 1));
    auto* align = dynamic_cast<embx::ir::Align*>(ms[5].get());
    REQUIRE((align && align->alignment));
    auto* cb = dynamic_cast<embx::ir::Callback*>(ms[6].get());
    REQUIRE((cb && cb->name == "on_decode" && cb->args.size() == 2));

    std::remove(path);
    std::cout << "IR structural tests passed\n";
}
