#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"

using Value = embx::value::Value;
using Object = Value::Object;
using Array = Value::Array;

static Value u(uint64_t x){ return Value{x}; }

static std::unique_ptr<embx::plan::Expr> id(const char* s){
    auto e=std::make_unique<embx::plan::Expr>(); e->kind=embx::core::ExprKind::Identifier; e->text=s; return e;
}
static embx::plan::Type prim(const char* n){ embx::plan::Type t; t.kind=embx::core::TypeKind::Primitive; t.name=n; return t; }

TEST_CASE("dynamic layout", "[dynamic_layout]") {
    embx::plan::Module p;
    embx::plan::Struct s; s.name="Dynamic";

    auto count=std::make_unique<embx::plan::Field>();
    count->name="count"; count->type=prim("u8");
    const auto countId = embx::test::addFieldSymbol(p, *count);
    s.members.push_back(std::move(count));

    auto values=std::make_unique<embx::plan::Field>();
    values->name="values"; values->type=prim("u16");
    auto countRef = id("count"); countRef->reference.id = countId;
    values->type.dimensions.push_back(embx::core::Dimension::fixed(std::move(countRef)));
    s.members.push_back(std::move(values));

    auto size=std::make_unique<embx::plan::Field>();
    size->name="block_size"; size->type=prim("u8");
    const auto sizeId = embx::test::addFieldSymbol(p, *size);
    s.members.push_back(std::move(size));

    auto block=std::make_unique<embx::plan::Block>();
    block->name="body"; block->size=id("block_size"); block->size->reference.id = sizeId;
    auto first=std::make_unique<embx::plan::Field>(); first->name="prefix"; first->type=prim("u8");
    block->members.push_back(std::move(first));
    auto rest=std::make_unique<embx::plan::Field>(); rest->name="rest"; rest->type=prim("bytes");
    rest->type.dimensions.push_back(embx::core::Dimension::remaining());
    block->members.push_back(std::move(rest));
    s.members.push_back(std::move(block));

    auto offset=std::make_unique<embx::plan::Field>(); offset->name="offset"; offset->type=prim("u8");
    const auto offsetId = embx::test::addFieldSymbol(p, *offset);
    // The field must precede the at() expression to be available at runtime.
    s.members.insert(s.members.begin()+3,std::move(offset));

    auto off=std::make_unique<embx::plan::At>(); off->offset=id("offset"); off->offset->reference.id = offsetId;
    auto target=std::make_unique<embx::plan::Field>(); target->name="target"; target->type=prim("u8");
    off->members.push_back(std::move(target));
    s.members.push_back(std::move(off));

    embx::test::addStruct(p, std::move(s));

    // count=2 => two u16 values. block_size=4 => prefix consumes one byte,
    // [*] must consume exactly the remaining three bytes of that bounded block.
    // offset=11 points to the final target byte and does not change the caller cursor.
    Object root{
        {"count",u(2)},
        {"values",Value{Array{u(0x1122),u(0x3344)}}},
        {"block_size",u(4)},
        {"prefix",u(0xAA)},
        {"rest",Value{Value::Bytes{0xBB,0xCC,0xDD}}},
        {"offset",u(11)},
        {"target",u(0xEE)}
    };

    embx::encoder::Engine enc(p); auto er=enc.encode("Dynamic",Value{root});
    REQUIRE(er.success);
    REQUIRE(er.data.size()==12);
    REQUIRE(er.data[0]==2);
    REQUIRE((er.data[1]==0x22 && er.data[2]==0x11));
    REQUIRE((er.data[3]==0x44 && er.data[4]==0x33));
    REQUIRE((er.data[5]==4 && er.data[6]==11));
    REQUIRE((er.data[7]==0xAA && er.data[8]==0xBB && er.data[9]==0xCC && er.data[10]==0xDD));
    REQUIRE(er.data[11]==0xEE);

    embx::decoder::Options opt; opt.requireFullInput=false;
    embx::decoder::Engine dec(p,opt); auto dr=dec.decode("Dynamic",er.data);
    REQUIRE((dr.success && dr.consumed==11));
    auto& o=std::get<Object>(dr.value.data);
    REQUIRE(std::get<uint64_t>(o.at("count").data)==2);
    auto& a=std::get<Array>(o.at("values").data); REQUIRE(a.size()==2);
    REQUIRE((std::get<uint64_t>(a[0].data)==0x1122 && std::get<uint64_t>(a[1].data)==0x3344));
    auto& restBytes=std::get<Value::Bytes>(o.at("rest").data); REQUIRE(restBytes==Value::Bytes({0xBB,0xCC,0xDD}));
    REQUIRE(std::get<uint64_t>(o.at("target").data)==0xEE);

    std::cout << "Dynamic layout tests passed\n";
}

TEST_CASE("runtime at preserves sequential cursor while align uses absolute region cursor") {
    const auto path = std::filesystem::temp_directory_path() / "embx_pass6_layout_boundary.embx";
    std::ofstream out(path);
    out << R"(struct S {
  head: u8;
  at($next + 2) {
    a: u8;
    align(4);
    b: u16;
  }
  tail: u8;
})";
    out.close();

    std::string err;
    auto ast = embx::parser::parseFile(path.string(), err);
    REQUIRE((ast && err.empty()));
    REQUIRE(embx::semantic::analyze(*ast, err));
    auto ir = embx::ir::lower(*ast, err);
    REQUIRE((ir && err.empty()));
    auto plan = embx::plan::build(*ir, err);
    REQUIRE((plan && err.empty()));

    Object input{{"head", u(7)}, {"a", u(0xAA)}, {"b", u(0x1122)}, {"tail", u(8)}};
    embx::encoder::Engine encoder(*plan);
    auto encoded = encoder.encode("S", Value{input});
    REQUIRE(encoded.success);
    REQUIRE(encoded.data == Value::Bytes{7, 8, 0, 0xAA, 0x22, 0x11});

    embx::decoder::Options options;
    options.requireFullInput = false;
    embx::decoder::Engine decoder(*plan, options);
    auto decoded = decoder.decode("S", encoded.data);
    REQUIRE(decoded.success);
    REQUIRE(decoded.consumed == 2);
    const auto& object = std::get<Object>(decoded.value.data);
    REQUIRE(std::get<uint64_t>(object.at("head").data) == 7);
    REQUIRE(std::get<uint64_t>(object.at("tail").data) == 8);
    REQUIRE(std::get<uint64_t>(object.at("a").data) == 0xAA);
    REQUIRE(std::get<uint64_t>(object.at("b").data) == 0x1122);

    std::filesystem::remove(path);
}
