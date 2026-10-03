#include <catch2/catch_test_macros.hpp>
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include <fstream>
#include <iostream>
#include <cstdio>
static bool check(const char* src,const char* needle,int n){std::string path="semantic_negative_"+std::to_string(n)+".embx";{std::ofstream f(path);f<<src;}std::string err;auto m=embx::parser::parseFile(path,err);if(!m){std::remove(path.c_str());return false;}bool ok=!embx::semantic::analyze(*m,err)&&err.find(needle)!=std::string::npos;std::remove(path.c_str());return ok;}
TEST_CASE("semantic negative", "[semantic_negative]") {
 REQUIRE(check("type A = Missing;","unknown type: Missing",1));
 REQUIRE(check("struct S { x: u8; x: u16; }","duplicate field: x",2));
 REQUIRE(check("struct S { bits { a: u8 (65); } }","invalid bit width",3));
 REQUIRE(check("struct S { bits { a: u8 (40); b: u8 (30); } }","exceeds 64",4));
 REQUIRE(check("struct S { variant v by x { 1: u8; 1: u16; } }","duplicate variant tag",5));
 REQUIRE(check("struct S { x: Missing; }","unknown type: Missing",6));
 REQUIRE(check("type A = B; type B = A;","cyclic type alias",7));
 REQUIRE(check("struct S { bits { a: bytes(8); } }","bit field requires integer type",8));
 REQUIRE(check("struct S { x: u8[*]; }","[*] is only valid for bytes/string",9));
 REQUIRE(check("struct S { x: bytes[2] [3]; }","array/length may be specified either",10));
 REQUIRE(check("struct S { x: u8 (3); }","bit width is only valid inside a bits block",11));
 REQUIRE(check("struct S { bits { a: u8 (3) [2]; } }","bit field cannot be an array",12));
 REQUIRE(check("struct S { bits { a: u8 (3) little; } }","bit field cannot have field modifiers",13));
 REQUIRE(check("struct S { x: bytes[$next]; }","$next is only valid in at(...) offset expressions",14));
 REQUIRE(check("struct S { x: u8; block body[$next] { y: u8; } }","$next is only valid in at(...) offset expressions",15));
 std::cout<<"Semantic negative tests passed\n";
}
