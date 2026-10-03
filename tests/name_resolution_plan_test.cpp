#include <catch2/catch_test_macros.hpp>
#include "TestPlan.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "plan/LayoutGraph.h"
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include <fstream>
#include <cstdio>

TEST_CASE("compiler plans require SymbolId references") {
    embx::plan::Module p;
    embx::plan::Struct leaf; leaf.name = "Leaf";
    auto lf = std::make_unique<embx::plan::Field>(); lf->name = "x";
    lf->type.kind = embx::core::TypeKind::Primitive; lf->type.name = "u8";
    leaf.members.push_back(std::move(lf));
    const auto leafId = embx::test::addStruct(p, std::move(leaf));

    embx::plan::Struct root; root.name = "Root";
    auto f = std::make_unique<embx::plan::Field>(); f->name = "leaf";
    f->type.kind = embx::core::TypeKind::Named; f->type.name = "Leaf"; f->type.reference.id = leafId;
    root.members.push_back(std::move(f));
    const auto rootId = embx::test::addStruct(p, std::move(root));
    REQUIRE(rootId != embx::core::InvalidSymbolId);

    std::string err; embx::plan::LayoutGraph g;
    REQUIRE(embx::plan::buildLayoutGraph(p, g, err));
    embx::value::Value::Object leafValue{{"x", embx::value::Value{uint64_t(7)}}};
    embx::value::Value::Object rootValue{{"leaf", embx::value::Value{std::move(leafValue)}}};
    embx::encoder::Engine enc(p);
    auto er = enc.encode("Root", embx::value::Value{std::move(rootValue)});
    REQUIRE(er.success);
    embx::decoder::Engine dec(p);
    auto dr = dec.decode("Root", er.data);
    REQUIRE(dr.success);
}


TEST_CASE("unqualified module values remain lexical and can be shadowed by fields") {
    const std::string path = "name_resolution_shadowing.embx";
    {
        std::ofstream out(path);
        out << "namespace app;\n"
               "const SIZE = 4;\n"
               "param count: u8;\n"
               "struct Packet {\n"
               "  SIZE: u8;\n"
               "  count: u8;\n"
               "  data: u8[SIZE];\n"
               "  more: u8[count];\n"
               "}\n";
    }
    std::string err;
    auto ast = embx::parser::parseFile(path, err);
    std::remove(path.c_str());
    REQUIRE(ast);
    REQUIRE(err.empty());
    REQUIRE(embx::semantic::analyze(*ast, err));
    REQUIRE(err.empty());
    auto ir = embx::ir::lower(*ast, err);
    REQUIRE(ir);
    REQUIRE(err.empty());

    const auto moduleSize = ir->symbolTable.findId("app::SIZE");
    const auto moduleCount = ir->symbolTable.findId("app::count");
    REQUIRE(moduleSize != embx::core::InvalidSymbolId);
    REQUIRE(moduleCount != embx::core::InvalidSymbolId);

    auto* packet = ir->structs.front().get();
    REQUIRE(packet);
    auto* sizeField = dynamic_cast<embx::ir::Field*>(packet->members[0].get());
    auto* countField = dynamic_cast<embx::ir::Field*>(packet->members[1].get());
    auto* data = dynamic_cast<embx::ir::Field*>(packet->members[2].get());
    auto* more = dynamic_cast<embx::ir::Field*>(packet->members[3].get());
    REQUIRE(sizeField);
    REQUIRE(countField);
    REQUIRE(data);
    REQUIRE(more);
    REQUIRE(data->type.dimensions.size() == 1);
    REQUIRE(more->type.dimensions.size() == 1);
    REQUIRE(data->type.dimensions[0].expression->reference.id == sizeField->symbol);
    REQUIRE(more->type.dimensions[0].expression->reference.id == countField->symbol);
    REQUIRE(data->type.dimensions[0].expression->reference.id != moduleSize);
    REQUIRE(more->type.dimensions[0].expression->reference.id != moduleCount);
}

TEST_CASE("qualified module values bypass lexical shadowing") {
    const std::string path = "name_resolution_qualified_shadowing.embx";
    {
        std::ofstream out(path);
        out << "namespace app;\n"
               "const SIZE = 4;\n"
               "struct Packet {\n"
               "  SIZE: u8;\n"
               "  data: u8[app::SIZE];\n"
               "}\n";
    }
    std::string err;
    auto ast = embx::parser::parseFile(path, err);
    std::remove(path.c_str());
    REQUIRE(ast);
    REQUIRE(err.empty());
    REQUIRE(embx::semantic::analyze(*ast, err));
    REQUIRE(err.empty());
    auto ir = embx::ir::lower(*ast, err);
    REQUIRE(ir);
    REQUIRE(err.empty());

    const auto moduleSize = ir->symbolTable.findId("app::SIZE");
    REQUIRE(moduleSize != embx::core::InvalidSymbolId);
    auto* data = dynamic_cast<embx::ir::Field*>(ir->structs.front()->members[1].get());
    REQUIRE(data);
    REQUIRE(data->type.dimensions.size() == 1);
    REQUIRE(data->type.dimensions[0].expression->reference.id == moduleSize);
}
