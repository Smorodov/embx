#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include <cstdint>
#include <iostream>

using Value = embx::value::Value;
using Object=Value::Object;

static const Value& field(const Value& v,const char* n){return std::get<Object>(v.data).at(n);}
static Value u(uint64_t x){return Value{x};}

TEST_CASE("encoder", "[encoder]") {
    embx::plan::Module p;
    embx::plan::Struct s; s.name="Packet";

    auto a=std::make_unique<embx::plan::Field>(); a->name="a"; a->type.name="u16"; a->type.kind=embx::core::TypeKind::Primitive; a->endian=embx::plan::Endian::Big; s.members.push_back(std::move(a));
    auto n=std::make_unique<embx::plan::Field>(); n->name="count"; n->type.name="u8"; n->type.kind=embx::core::TypeKind::Primitive; s.members.push_back(std::move(n));
    auto arr=std::make_unique<embx::plan::Field>(); arr->name="values"; arr->type.name="u16"; arr->type.kind=embx::core::TypeKind::Primitive; auto ae=std::make_unique<embx::plan::Expr>(); ae->kind=embx::core::ExprKind::Literal; ae->text="2"; arr->type.dimensions.push_back(embx::core::Dimension::fixed(std::move(ae))); s.members.push_back(std::move(arr));
    auto bits=std::make_unique<embx::plan::Bits>(); bits->totalBits=8; bits->storageBytes=1; embx::plan::BitsField hi; hi.name="hi"; hi.bits=3; hi.type.name="u8"; hi.type.kind=embx::core::TypeKind::Primitive; bits->fields.push_back(std::move(hi)); embx::plan::BitsField lo; lo.name="lo"; lo.bits=5; lo.type.name="u8"; lo.type.kind=embx::core::TypeKind::Primitive; bits->fields.push_back(std::move(lo)); s.members.push_back(std::move(bits));
    auto block=std::make_unique<embx::plan::Block>(); block->name="payload"; block->staticSize=2; auto bf=std::make_unique<embx::plan::Field>(); bf->name="x"; bf->type.name="u16"; bf->type.kind=embx::core::TypeKind::Primitive; bf->endian=embx::plan::Endian::Big; block->members.push_back(std::move(bf)); s.members.push_back(std::move(block));
    embx::test::addStruct(p, std::move(s));

    Object root;
    root["a"]=u(0x1234); root["count"]=u(2);
    Value::Array va; va.emplace_back(u(0x0201)); va.emplace_back(u(0x0403)); root["values"]=std::move(va);
    root["hi"]=u(5); root["lo"]=u(5);
    root["x"]=u(7);

    embx::encoder::Engine e(p); auto r=e.encode("Packet",Value{root}); REQUIRE(r.success);
    const std::vector<uint8_t> expected={0x12,0x34,2,0x01,0x02,0x03,0x04,0xA5,0x00,0x07}; REQUIRE(r.data==expected);

    embx::decoder::Options dop; dop.requireFullInput=true; embx::decoder::Engine d(p,dop); auto dr=d.decode("Packet",r.data); REQUIRE((dr.success&&dr.consumed==r.data.size()));
    REQUIRE(std::get<uint64_t>(field(dr.value,"a").data)==0x1234);

    std::cout<<"Encoder tests passed\n";
}
