#include <catch2/catch_test_macros.hpp>
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include <cstdio>
#include <fstream>
#include <string>

static void writeFile(const char* path, const char* text) {
    std::ofstream f(path, std::ios::binary); f << text;
}

TEST_CASE("namespace import", "[namespace_import]") {
    writeFile("embx17_common.embx",
        "namespace common::net;\n"
        "const SIZE = 4;\n"
        "struct Header { value: u16; }\n");
    writeFile("embx17_payload.embx",
        "namespace payload;\n"
        "struct Data { value: u8; }\n");
    writeFile("embx17_main.embx",
        "namespace app;\n"
        "import \"embx17_common.embx\";\n"
        "import \"embx17_payload.embx\" as p;\n"
        "type H = common::net::Header;\n"
        "const LOCAL_SIZE = 3;\n"
        "struct Packet { header: H; body: p::Data; n: u8[common::net::SIZE]; local: u8[LOCAL_SIZE]; }\n");

    std::string err;
    auto m = embx::parser::parseFile("embx17_main.embx", err);
    REQUIRE((m && err.empty()));
    REQUIRE(m->nameSpace == "app");

    bool foundHeader=false, foundData=false, foundPacket=false, foundAlias=false;
    for (const auto& s : m->structs) {
        foundHeader |= s->name == "common::net::Header";
        foundData |= s->name == "payload::Data";
        if (s->name == "app::Packet") {
            foundPacket=true;
            auto* f0=dynamic_cast<const embx::ast::Field*>(s->members[0].get());
            auto* f1=dynamic_cast<const embx::ast::Field*>(s->members[1].get());
            auto* f2=dynamic_cast<const embx::ast::Field*>(s->members[2].get());
            auto* f3=dynamic_cast<const embx::ast::Field*>(s->members[3].get());
            REQUIRE((f0 && f0->type.name == "app::H"));
            REQUIRE((f1 && f1->type.name == "payload::Data"));
            REQUIRE((f2 && f2->type.suffixes.size()==1 && f2->type.suffixes[0].expr));
            REQUIRE(f2->type.suffixes[0].expr->text == "common::net::SIZE");
            REQUIRE((f3 && f3->type.suffixes.size()==1 && f3->type.suffixes[0].expr));
            REQUIRE(f3->type.suffixes[0].expr->text == "LOCAL_SIZE");
        }
    }
    for (const auto& a : m->aliases) if (a.name == "app::H" && a.target.name == "common::net::Header") foundAlias=true;
    REQUIRE((foundHeader && foundData && foundPacket && foundAlias));
    REQUIRE(embx::semantic::analyze(*m, err));
    auto ir=embx::ir::lower(*m,err); REQUIRE((ir && err.empty()));
    REQUIRE(ir->symbolTable.findId("app::Packet") != embx::core::InvalidSymbolId);
    REQUIRE(ir->symbolTable.findId("common::net::Header") != embx::core::InvalidSymbolId);

    writeFile("embx17_a.embx", "namespace a; import \"embx17_b.embx\"; struct A { x: u8; }\n");
    writeFile("embx17_b.embx", "namespace b; import \"embx17_a.embx\"; struct B { x: u8; }\n");
    auto cyc=embx::parser::parseFile("embx17_a.embx",err);
    REQUIRE((!cyc && err.find("import cycle") != std::string::npos));

    std::remove("embx17_common.embx"); std::remove("embx17_payload.embx"); std::remove("embx17_main.embx");
    std::remove("embx17_a.embx"); std::remove("embx17_b.embx");
}
