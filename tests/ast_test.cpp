#include <catch2/catch_test_macros.hpp>
#include "ast/Ast.h"
#include <iostream>
using namespace embx::ast;
static std::unique_ptr<Expr> id(const char* s){auto e=std::make_unique<Expr>();e->kind=ExprKind::Identifier;e->text=s;return e;}
TEST_CASE("ast", "[ast]") {
 Module m; m.endian=Endian::Big;
 TypeAlias a; a.name="Payload"; a.target.name="bytes"; TypeSuffix sx; sx.dynamic=true; a.target.suffixes.push_back(std::move(sx)); m.aliases.push_back(std::move(a));
 Const c; c.name="N"; c.expr=id("length"); m.constants.push_back(std::move(c));
 auto s=std::make_unique<Struct>(); s->name="S";
 auto f=std::make_unique<Field>(); f->name="data"; f->type.name="bytes"; TypeSuffix sl; sl.expr=id("N+1"); f->type.suffixes.push_back(std::move(sl)); FieldModifier fm; fm.kind=ModifierKind::Length; fm.expr=id("N"); f->modifiers.push_back(std::move(fm)); s->members.push_back(std::move(f));
 auto bits=std::make_unique<Bits>(); auto bf=std::make_unique<Field>(); bf->name="lo"; bf->type.name="u8"; bf->bits=4; bits->fields.push_back(std::move(bf)); s->members.push_back(std::move(bits));
 auto v=std::make_unique<Variant>(); v->name="v"; v->discriminator=id("kind"); VariantCase vc; vc.tag=id("1"); vc.type=std::make_unique<TypeRef>(); vc.type->name="Payload"; v->cases.push_back(std::move(vc)); v->defaultType=std::make_unique<TypeRef>(); v->defaultType->name="bytes"; s->members.push_back(std::move(v));
 auto b=std::make_unique<Block>(); b->name="payload"; b->size=id("N"); s->members.push_back(std::move(b));
 s->members.push_back(std::make_unique<Align>(id("4")));
 auto at=std::make_unique<At>(); at->offset=id("64"); s->members.push_back(std::move(at));
 auto cb=std::make_unique<Callback>(); cb->name="on_encode_decompress"; cb->direction=CallbackDirection::Encode; cb->args.push_back(id("size")); s->members.push_back(std::move(cb));
 m.structs.push_back(std::move(s));
 REQUIRE((m.aliases.size()==1 && m.aliases[0].target.suffixes[0].dynamic));
 REQUIRE((m.structs.size()==1 && m.structs[0]->members.size()==7));
 REQUIRE(dynamic_cast<Bits*>(m.structs[0]->members[1].get()));
 auto* vv=dynamic_cast<Variant*>(m.structs[0]->members[2].get()); REQUIRE((vv&&vv->cases[0].type&&vv->cases[0].type->name=="Payload"&&vv->defaultType));
 REQUIRE(dynamic_cast<Block*>(m.structs[0]->members[3].get()));
 REQUIRE(dynamic_cast<Align*>(m.structs[0]->members[4].get()));
 REQUIRE(dynamic_cast<At*>(m.structs[0]->members[5].get()));
 REQUIRE(dynamic_cast<Callback*>(m.structs[0]->members[6].get()));
 std::cout<<"AST structural tests passed\n";
}
