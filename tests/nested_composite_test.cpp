#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"
#include <cstdint>
#include <iostream>

using Value = embx::value::Value;
using Object = Value::Object;
using Array = Value::Array;

static Value u(uint64_t x){ return Value{x}; }
static std::unique_ptr<embx::plan::Expr> lit(const char* s){
    auto e=std::make_unique<embx::plan::Expr>(); e->kind=embx::core::ExprKind::Literal; e->text=s; return e;
}
static std::unique_ptr<embx::plan::Expr> id(const char* s){
    auto e=std::make_unique<embx::plan::Expr>(); e->kind=embx::core::ExprKind::Identifier; e->text=s; return e;
}
static embx::plan::Type prim(const char* n){ embx::plan::Type t; t.kind=embx::core::TypeKind::Primitive; t.name=n; return t; }
static embx::plan::Type named(const char* n, embx::core::SymbolId id){ embx::plan::Type t; t.kind=embx::core::TypeKind::Named; t.name=n; t.reference.id=id; return t; }
static void field(embx::plan::Struct& s,const char* n,embx::plan::Type t){
    auto f=std::make_unique<embx::plan::Field>(); f->name=n; f->type=std::move(t); s.members.push_back(std::move(f));
}

TEST_CASE("nested composite", "[nested_composite]") {
    embx::plan::Module p;

    embx::plan::Struct leaf; leaf.name="Leaf";
    field(leaf,"code",prim("u16")); field(leaf,"value",prim("u8"));
    const auto leafId = embx::test::addStruct(p, std::move(leaf));

    embx::plan::Struct packet; packet.name="Packet";
    field(packet,"head",named("Leaf", leafId));
    {
        auto f=std::make_unique<embx::plan::Field>(); f->name="items"; f->type=named("Leaf", leafId);
        f->type.dimensions.push_back(embx::core::Dimension::fixed(lit("2"))); packet.members.push_back(std::move(f));
    }
    {
        auto b=std::make_unique<embx::plan::Block>(); b->name="body"; b->staticSize=3;
        auto f=std::make_unique<embx::plan::Field>(); f->name="inner"; f->type=named("Leaf", leafId);
        b->members.push_back(std::move(f)); packet.members.push_back(std::move(b));
    }
    {
        auto a=std::make_unique<embx::plan::At>(); a->staticOffset=20;
        auto f=std::make_unique<embx::plan::Field>(); f->name="tail"; f->type=named("Leaf", leafId);
        a->members.push_back(std::move(f)); packet.members.push_back(std::move(a));
    }
    {
        auto kind=std::make_unique<embx::plan::Field>(); kind->name="kind"; kind->type=prim("u8");
        const auto kindId = embx::test::addFieldSymbol(p, *kind); packet.members.push_back(std::move(kind));
        auto v=std::make_unique<embx::plan::Variant>(); v->name="choice"; v->discriminator=id("kind"); v->discriminator->reference.id = kindId;
        embx::plan::VariantCase c; c.tag=std::uint64_t(1);
        auto f=std::make_unique<embx::plan::Field>(); f->name="payload"; f->type=named("Leaf", leafId); c.members.push_back(std::move(f));
        v->cases.push_back(std::move(c)); packet.members.push_back(std::move(v));
    }
    embx::test::addStruct(p, std::move(packet));

    auto leafValue=[](uint64_t c,uint64_t v){ return Value{Object{{"code",u(c)},{"value",u(v)}}}; };
    Array items{leafValue(2,20),leafValue(3,30)};
    Object root{
        {"head",leafValue(1,10)}, {"items",items}, {"inner",leafValue(4,40)},
        {"tail",leafValue(5,50)}, {"kind",u(1)},
        {"choice",Value{Object{{"payload",leafValue(6,60)}}}}
    };

    embx::encoder::Engine enc(p); auto er=enc.encode("Packet",Value{root}); if(!er.success){std::cerr<<er.error<<" path="<<er.diagnostic.path<<"\n";} REQUIRE(er.success);
    REQUIRE(er.data.size() >= 23);
    REQUIRE((er.data[0]==1 && er.data[1]==0 && er.data[2]==10));
    REQUIRE((er.data[3]==2 && er.data[4]==0 && er.data[5]==20));
    REQUIRE((er.data[6]==3 && er.data[7]==0 && er.data[8]==30));
    REQUIRE((er.data[20]==5 && er.data[21]==0 && er.data[22]==50));

    embx::decoder::Options opt; opt.requireFullInput=false;
    embx::decoder::Engine dec(p,opt); auto dr=dec.decode("Packet",er.data); REQUIRE(dr.success);
    auto& o=std::get<Object>(dr.value.data);
    REQUIRE(std::get<uint64_t>(std::get<Object>(o.at("head").data).at("value").data)==10);
    auto& ia=std::get<Array>(o.at("items").data); REQUIRE(ia.size()==2);
    REQUIRE(std::get<uint64_t>(std::get<Object>(ia[1].data).at("code").data)==3);
    REQUIRE(std::get<uint64_t>(std::get<Object>(o.at("inner").data).at("code").data)==4);
    REQUIRE(std::get<uint64_t>(std::get<Object>(o.at("tail").data).at("value").data)==50);
    REQUIRE(std::get<Object>(o.at("choice").data).count("payload")==1);

    auto bad=er.data; bad.resize(8);
    auto fail=dec.decode("Packet",bad);
    REQUIRE(!fail.success);
    REQUIRE(fail.diagnostic.path=="Packet.items[1].value");

    std::cout << "Nested composite tests passed\n";
}
