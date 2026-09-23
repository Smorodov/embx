#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "reflection/Reflection.h"
namespace { std::shared_ptr<embx::plan::Module> makePlan(const std::string& source,std::string& err){const std::string path="field_alias_test.embx";{std::ofstream out(path);out<<source;}auto ast=embx::parser::parseFile(path,err);std::remove(path.c_str());if(!ast)return{};if(!embx::semantic::analyze(*ast,err))return{};auto ir=embx::ir::lower(*ast,err);if(!ir)return{};auto p=embx::plan::build(*ir,err);if(!p)return{};return std::shared_ptr<embx::plan::Module>(std::move(p));}}
TEST_CASE("read-only field alias consumes no bytes"){std::string err;auto p=makePlan(R"(struct Packet { value: u8; alias value2 = value; })",err);REQUIRE(p);REQUIRE(err.empty());embx::value::Value::Object in;in["value"]=uint64_t(7);embx::encoder::Engine enc(*p);auto e=enc.encode("Packet",in);REQUIRE(e);REQUIRE(e.data==std::vector<uint8_t>{7});embx::decoder::Engine dec(*p);auto d=dec.decode("Packet",e.data);REQUIRE(d);REQUIRE(d.value.get("value2"));REQUIRE(std::get<uint64_t>(d.value.at("value2").data)==7);}
TEST_CASE("field alias can drive a later expression"){std::string err;auto p=makePlan(R"(struct Packet { value: u8; alias value2 = value; payload: bytes[value2]; })",err);REQUIRE(p);REQUIRE(err.empty());embx::value::Value::Object in;in["value"]=uint64_t(2);in["payload"]=embx::value::Value::Bytes{1,2};embx::encoder::Engine enc(*p);auto e=enc.encode("Packet",in);REQUIRE(e);REQUIRE(e.data==std::vector<uint8_t>{2,1,2});}
TEST_CASE("field alias cannot reference a later member"){std::string err;auto p=makePlan(R"(struct Packet { alias value2 = value; value: u8; })",err);REQUIRE_FALSE(p);REQUIRE(err.find("declared later")!=std::string::npos);}

TEST_CASE("field alias is a writable input for its target"){std::string err;auto p=makePlan(R"(struct Packet { value: u8; alias value2 = value; })",err);REQUIRE(p);REQUIRE(err.empty());embx::value::Value::Object in;in["value2"]=uint64_t(9);embx::encoder::Engine enc(*p);auto e=enc.encode("Packet",in);REQUIRE(e);REQUIRE(e.data==std::vector<uint8_t>{9});}
TEST_CASE("field alias and target must agree when both are supplied"){std::string err;auto p=makePlan(R"(struct Packet { value: u8; alias value2 = value; })",err);REQUIRE(p);embx::value::Value::Object in;in["value"]=uint64_t(7);in["value2"]=uint64_t(8);embx::encoder::Engine enc(*p);auto e=enc.encode("Packet",in);REQUIRE_FALSE(e);REQUIRE(e.error.find("disagree")!=std::string::npos);}
TEST_CASE("field alias cannot write through a virtual target"){std::string err;auto p=makePlan(R"(struct Packet { value: u8; let computed = value; alias value2 = computed; })",err);REQUIRE_FALSE(p);REQUIRE_FALSE(err.empty());}
TEST_CASE("field alias appears in reflection"){std::string err;auto p=makePlan(R"(struct Packet { value: u8; alias value2 = value; })",err);REQUIRE(p);auto s=embx::reflection::inspectStruct(*p,"Packet");REQUIRE(s);REQUIRE(s->members.size()==2);REQUIRE(s->members[1].kind==embx::reflection::Kind::Alias);REQUIRE(s->members[1].name=="value2");REQUIRE(s->members[1].children==std::vector<std::string>{"value"});}
