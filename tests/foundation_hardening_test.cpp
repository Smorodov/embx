#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"
#include "reflection/Reflection.h"
#include <cstdint>
#include <iostream>

using V = embx::value::Value;
using Obj = V::Object;
static std::unique_ptr<embx::plan::Expr> lit(const char* x){auto e=std::make_unique<embx::plan::Expr>();e->kind=embx::core::ExprKind::Literal;e->text=x;return e;}
static std::unique_ptr<embx::plan::Field> field(const char* n,const char* t){auto f=std::make_unique<embx::plan::Field>();f->name=n;f->type.name=t;f->type.kind=embx::core::TypeKind::Primitive;return f;}
static void add(embx::plan::Module& p, embx::plan::Struct s){embx::test::addStruct(p, std::move(s));}

TEST_CASE("foundation hardening", "[foundation_hardening]") {
    // Assertions, zero-length arrays, enum runtime and root-relative at().
    embx::plan::Module p;
    embx::plan::Enum en; en.name="Kind"; en.underlying.kind=embx::core::TypeKind::Primitive; en.underlying.name="u8"; en.values["A"]=uint64_t(7); const auto kindId = embx::test::addEnum(p, std::move(en));
    embx::plan::Struct s; s.name="Inner";
    auto x=field("x","u8"); x->assertion="7"; s.members.push_back(std::move(x)); add(p,std::move(s));
    embx::plan::Struct root; root.name="Root";
    auto pre=field("pre","u8"); pre->assertion="7"; root.members.push_back(std::move(pre));
    auto e=field("kind","Kind"); e->type.kind=embx::core::TypeKind::Named; e->type.reference.id=kindId; root.members.push_back(std::move(e));
    auto z=field("empty","u8"); auto ze=lit("0"); z->type.dimensions.push_back(embx::core::Dimension::fixed(std::move(ze))); root.members.push_back(std::move(z));
    auto at=std::make_unique<embx::plan::At>(); at->offset=lit("0"); auto ax=field("absolute","u8"); at->members.push_back(std::move(ax)); root.members.push_back(std::move(at));
    add(p,std::move(root));

    embx::decoder::Options dop; dop.requireFullInput=true; dop.maxAllocationBytes=1024; dop.maxRecursionDepth=8;
    embx::decoder::Engine d(p,dop);
    auto r=d.decode("Root",{7,7}); if(!r.success) FAIL("decode failed: " << r.error); REQUIRE(r.consumed==2);
    const Obj& ro=std::get<Obj>(r.value.data); REQUIRE(std::get<uint64_t>(ro.at("kind").data)==7); REQUIRE(std::get<V::Array>(ro.at("empty").data).empty()); REQUIRE(std::get<uint64_t>(ro.at("absolute").data)==7);
    auto bad=d.decode("Root",{6,7}); REQUIRE((!bad.success && bad.error.find("assertion mismatch")!=std::string::npos));

    // Encoder enforces the same assertion and enum representation.
    Obj vo; vo["pre"]=uint64_t(7); vo["kind"]=uint64_t(7); vo["empty"]=V::Array{}; vo["absolute"]=uint64_t(7);
    embx::encoder::Engine eeng(p); auto er=eeng.encode("Root",V{vo}); REQUIRE((er.success && er.data==std::vector<uint8_t>({7,7})));
    vo["pre"]=uint64_t(6); auto eb=eeng.encode("Root",V{vo}); REQUIRE((!eb.success && eb.error.find("assertion mismatch")!=std::string::npos));

    // Root-relative at() inside a nested struct: offset 0 must target byte 0, not nested start.
    embx::plan::Module q;
    embx::plan::Struct in; in.name="In"; auto a=std::make_unique<embx::plan::At>(); a->offset=lit("0"); a->members.push_back(field("v","u8")); in.members.push_back(std::move(a)); const auto inId = embx::test::addStruct(q,std::move(in));
    embx::plan::Struct out; out.name="Out"; out.members.push_back(field("prefix","u8")); auto n=field("in","In"); n->type.kind=embx::core::TypeKind::Named; n->type.reference.id=inId; out.members.push_back(std::move(n)); add(q,std::move(out));
    embx::decoder::Engine qd(q); auto qr=qd.decode("Out",{9,8}); REQUIRE(qr.success); const Obj& qo=std::get<Obj>(qr.value.data); REQUIRE(std::get<uint64_t>(qo.at("prefix").data)==9); const Obj& qi=std::get<Obj>(qo.at("in").data); REQUIRE(std::get<uint64_t>(qi.at("v").data)==9);

    // Failed root-relative at() must be transactional even when it overwrites existing bytes.
    embx::plan::Module tx; embx::plan::Struct ts; ts.name="TransactionalAt";
    ts.members.push_back(field("prefix","u8"));
    auto ta=std::make_unique<embx::plan::At>(); ta->offset=lit("0");
    ta->members.push_back(field("replacement","u8"));
    ta->members.push_back(field("missing","u8"));
    ts.members.push_back(std::move(ta)); add(tx,std::move(ts));
    embx::value::Value::Object tvo; tvo["prefix"]=uint64_t(9); tvo["replacement"]=uint64_t(3);
    auto txr=embx::encoder::Engine(tx).encode("TransactionalAt",V{tvo});
    REQUIRE_FALSE(txr.success);
    REQUIRE(txr.data.empty());

    // String and bytes assertions, including negative signed integer literals.
    embx::plan::Module ap; embx::plan::Struct as; as.name="Assertions";
    auto si=field("s","string"); si->type.dimensions.push_back(embx::core::Dimension::fixed(lit("3"))); si->assertion="\"abc\""; as.members.push_back(std::move(si));
    auto by=field("b","bytes"); by->type.dimensions.push_back(embx::core::Dimension::fixed(lit("2"))); by->assertion="\"XY\""; as.members.push_back(std::move(by));
    auto neg=field("n","i8"); neg->assertion="-1"; as.members.push_back(std::move(neg)); add(ap,std::move(as));
    embx::decoder::Engine ad(ap); auto ar=ad.decode("Assertions",{'a','b','c','X','Y',255}); REQUIRE(ar.success);
    auto arbad=ad.decode("Assertions",{'a','b','d','X','Y',255}); REQUIRE((!arbad.success && arbad.error.find("assertion mismatch")!=std::string::npos));

    // Explicit recursion guard on a cyclic plan.
    embx::plan::Module cyc; embx::plan::Struct cs; cs.name="Node"; const auto nodeId = embx::test::addStruct(cyc,std::move(cs)); auto nx=field("next","Node"); nx->type.kind=embx::core::TypeKind::Named; nx->type.reference.id=nodeId; embx::test::addFieldSymbol(cyc, *nx); cyc.structs.front().members.push_back(std::move(nx)); embx::decoder::Options co; co.maxRecursionDepth=3; embx::decoder::Engine cd(cyc,co); auto cr=cd.decode("Node",{}); REQUIRE((!cr.success && cr.error.find("maximum recursion depth")!=std::string::npos));

    // Reflection keeps composite hierarchy.
    auto rs=embx::reflection::inspectStruct(p,"Root"); REQUIRE(rs); bool sawAt=false; for(const auto& m:rs->members) if(m.kind==embx::reflection::Kind::At){sawAt=true;REQUIRE((m.nested.size()==1&&m.nested[0].name=="absolute"));} REQUIRE(sawAt);
    std::cout<<"Foundation hardening tests passed\n";
}
