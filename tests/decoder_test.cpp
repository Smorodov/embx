#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "decoder/Decoder.h"
#include <cstdint>
#include <iostream>

using Value = embx::value::Value;

static uint64_t u(const Value& v){ return std::get<uint64_t>(v.data); }
static const Value& field(const Value& v,const char* n){ return std::get<Value::Object>(v.data).at(n); }

TEST_CASE("decoder", "[decoder]") {
    embx::plan::Module p;
    p.endian=embx::plan::Endian::Little;
    embx::plan::Struct s; s.name="Packet";

    auto a=std::make_unique<embx::plan::Field>(); a->name="a"; a->type.name="u16"; a->type.kind=embx::core::TypeKind::Primitive; a->endian=embx::plan::Endian::Big; s.members.push_back(std::move(a));
    auto n=std::make_unique<embx::plan::Field>(); n->name="count"; n->type.name="u8"; n->type.kind=embx::core::TypeKind::Primitive; s.members.push_back(std::move(n));
    auto arr=std::make_unique<embx::plan::Field>(); arr->name="values"; arr->type.name="u16"; arr->type.kind=embx::core::TypeKind::Primitive; auto ae=std::make_unique<embx::plan::Expr>(); ae->kind=embx::core::ExprKind::Literal; ae->text="2"; arr->type.dimensions.push_back(embx::core::Dimension::fixed(std::move(ae))); s.members.push_back(std::move(arr));
    auto bits=std::make_unique<embx::plan::Bits>(); bits->totalBits=8; bits->storageBytes=1; embx::plan::BitsField bx; bx.name="hi"; bx.bits=3; bx.type.name="u8"; bx.type.kind=embx::core::TypeKind::Primitive; bits->fields.push_back(std::move(bx)); embx::plan::BitsField by; by.name="lo"; by.bits=5; by.type.name="u8"; by.type.kind=embx::core::TypeKind::Primitive; bits->fields.push_back(std::move(by)); s.members.push_back(std::move(bits));

    auto block=std::make_unique<embx::plan::Block>(); block->name="payload"; block->staticSize=2; auto bf=std::make_unique<embx::plan::Field>(); bf->name="x"; bf->type.name="u16"; bf->type.kind=embx::core::TypeKind::Primitive; bf->endian=embx::plan::Endian::Big; block->members.push_back(std::move(bf)); s.members.push_back(std::move(block));
    embx::test::addStruct(p, std::move(s));

    std::vector<uint8_t> data={0x12,0x34, 2, 0x01,0x02,0x03,0x04, 0xA5, 0x00,0x07};
    embx::decoder::Options opt; opt.requireFullInput=true; embx::decoder::Engine e(p, opt); auto r=e.decode("Packet",data); REQUIRE(r.success); REQUIRE(r.consumed==data.size());
    REQUIRE(u(field(r.value,"a"))==0x1234); REQUIRE(u(field(r.value,"count"))==2);
    const auto& va=std::get<Value::Array>(field(r.value,"values").data); REQUIRE((va.size()==2&&u(va[0])==0x0201&&u(va[1])==0x0403));
    REQUIRE((u(field(r.value,"hi"))==5 && u(field(r.value,"lo"))==5));
    REQUIRE(u(field(r.value,"x"))==7);

    embx::plan::Module pv; embx::plan::Struct vs; vs.name="V";
    auto d=std::make_unique<embx::plan::Field>(); d->name="tag"; d->type.name="u8"; d->type.kind=embx::core::TypeKind::Primitive;
    const auto tagId = embx::test::addFieldSymbol(pv, *d); vs.members.push_back(std::move(d));
    auto v=std::make_unique<embx::plan::Variant>(); v->name="body"; v->discriminator=std::make_unique<embx::plan::Expr>(); v->discriminator->kind=embx::core::ExprKind::Identifier; v->discriminator->text="tag"; v->discriminator->reference.id=tagId;
    embx::plan::VariantCase c; c.tag=std::uint64_t(1); c.type=embx::plan::Type{}; c.type->name="u16"; c.type->kind=embx::core::TypeKind::Primitive; v->cases.push_back(std::move(c)); vs.members.push_back(std::move(v)); embx::test::addStruct(pv, std::move(vs));
    embx::decoder::Engine ve(pv, opt); auto vr=ve.decode("V",{1,0x78,0x56}); REQUIRE(vr.success); REQUIRE(u(field(vr.value,"tag"))==1); REQUIRE(u(field(field(vr.value,"body"),"value"))==0x5678);

    std::cout<<"Decoder tests passed\n";
}
