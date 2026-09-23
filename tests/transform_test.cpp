#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <fstream>
#include <cstdio>
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "reflection/Reflection.h"
namespace { std::shared_ptr<embx::plan::Module> plan(const std::string& s,std::string& e){std::ofstream f("transform_test.embx");f<<s;f.close();auto a=embx::parser::parseFile("transform_test.embx",e);std::remove("transform_test.embx");if(!a)return{};if(!embx::semantic::analyze(*a,e))return{};auto i=embx::ir::lower(*a,e);if(!i)return{};auto p=embx::plan::build(*i,e);if(!p)return{};return std::shared_ptr<embx::plan::Module>(std::move(p));}}
TEST_CASE("scale parses and reaches plan"){std::string e;auto p=plan("struct P { x: i16 transform scale(0.1); }",e);REQUIRE(p);auto id=p->symbolTable.findId("P");auto&s=p->structs[p->structIndexBySymbol.at(id)];auto*f=static_cast<embx::plan::Field*>(s.members[0].get());REQUIRE(f->transform);REQUIRE(f->transform->name=="scale");REQUIRE(f->transform->factor==Catch::Approx(0.1));}
TEST_CASE("scale decode and encode inverse"){std::string e;auto p=plan("struct P { x: i16 transform scale(0.1); }",e);REQUIRE(p);embx::decoder::Engine d(*p);auto r=d.decode("P",{253,0});REQUIRE(r);REQUIRE(std::get<double>(r.value.at("x").data)==Catch::Approx(25.3));embx::value::Value::Object o;o["x"]=25.3;auto q=embx::encoder::Engine(*p).encode("P",o);REQUIRE(q);REQUIRE(q.data==std::vector<uint8_t>{253,0});}
TEST_CASE("scale does not change physical layout"){std::string e;auto p=plan("struct P { a: u8; x: i16 transform scale(0.1); b: u8; }",e);REQUIRE(p);auto id=p->symbolTable.findId("P");auto&s=p->structs[p->structIndexBySymbol.at(id)];auto*x=static_cast<embx::plan::Field*>(s.members[1].get());REQUIRE(x->staticOffset==1);REQUIRE(x->staticSize==2);REQUIRE(s.staticSize==4);}
TEST_CASE("alias uses physical field transform"){std::string e;auto p=plan("struct P { x: i16 transform scale(0.1); alias temperature = x; }",e);REQUIRE(p);embx::value::Value::Object o;o["temperature"]=25.3;auto q=embx::encoder::Engine(*p).encode("P",o);REQUIRE(q);REQUIRE(q.data==std::vector<uint8_t>{253,0});}
TEST_CASE("transform is reflected as field metadata"){std::string e;auto p=plan("struct P { x: i16 transform scale(0.1); }",e);REQUIRE(p);auto r=embx::reflection::inspectStruct(*p,"P");REQUIRE(r);REQUIRE(r->members[0].transform=="scale");REQUIRE(r->members[0].transformFactor==Catch::Approx(0.1));}
TEST_CASE("unknown transform rejected"){std::string e;auto p=plan("struct P { x: i16 transform foo(1); }",e);REQUIRE_FALSE(p);REQUIRE(e.find("unknown transform")!=std::string::npos);}
TEST_CASE("scale requires one argument"){std::string e;auto p=plan("struct P { x: i16 transform scale(1,2); }",e);REQUIRE_FALSE(p);REQUIRE(e.find("exactly one")!=std::string::npos);}
TEST_CASE("scale rejects zero factor"){std::string e;auto p=plan("struct P { x: i16 transform scale(0); }",e);REQUIRE_FALSE(p);REQUIRE(e.find("non-zero")!=std::string::npos);}
TEST_CASE("scale rejects runtime parameter"){std::string e;auto p=plan("param k: f64; struct P { x: i16 transform scale(k); }",e);REQUIRE_FALSE(p);REQUIRE(e.find("compile-time constant")!=std::string::npos);}
TEST_CASE("scale rejects virtual and alias targets"){std::string e;auto p=plan("struct P { x: i16; let y = x; }",e);REQUIRE(p);e.clear();p=plan("struct P { x: i16; alias y = x; }",e);REQUIRE(p);}
TEST_CASE("scale rejects non numeric wire type"){std::string e;auto p=plan("struct P { x: bytes[2] transform scale(0.1); }",e);REQUIRE_FALSE(p);REQUIRE(e.find("scalar numeric")!=std::string::npos);}
TEST_CASE("scale encode rejects rounding"){std::string e;auto p=plan("struct P { x: i16 transform scale(0.1); }",e);REQUIRE(p);embx::value::Value::Object o;o["x"]=25.35;auto q=embx::encoder::Engine(*p).encode("P",o);REQUIRE_FALSE(q);}

TEST_CASE("scale rejects non-exact f32 narrowing"){
    std::string e; auto p=plan("struct P { x: f32 transform scale(1.0); }",e); REQUIRE(p);
    embx::value::Value::Object o; o["x"] = 16777217.0;
    auto q=embx::encoder::Engine(*p).encode("P",o); REQUIRE_FALSE(q);
}

TEST_CASE("scale rejects imprecise wide integer decode"){
    std::string e; auto p=plan("struct P { x: u64 transform scale(1.0); }",e); REQUIRE(p);
    embx::decoder::Engine d(*p);
    auto r=d.decode("P",{1,0,0,0,0,0,32,0});
    REQUIRE_FALSE(r);
}

TEST_CASE("scale accepts exact u64 boundary representable by double"){
    std::string e; auto p=plan("struct P { x: u64 transform scale(1.0); }",e); REQUIRE(p);
    embx::decoder::Engine d(*p);
    auto r=d.decode("P",{0,0,0,0,0,0,32,0});
    REQUIRE(r); REQUIRE(std::get<double>(r.value.at("x").data)==Catch::Approx(9007199254740992.0));
}
