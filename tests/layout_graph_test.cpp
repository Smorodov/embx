#include "TestPlan.h"
#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include "ir/IrBuilder.h"
#include "parser/ParserDriver.h"
#include "plan/LayoutGraph.h"
#include "plan/PlanBuilder.h"
#include "semantic/SemanticAnalyzer.h"
#include <cstdio>
#include <fstream>
#include <iostream>

static std::unique_ptr<embx::plan::Module> makePlan(const char* text, std::string& err) {
    const char* path = "layout_graph.embx";
    std::ofstream(path) << text;
    auto ast = embx::parser::parseFile(path, err);
    if (!ast) { std::remove(path); return {}; }
    if (!embx::semantic::analyze(*ast, err)) { std::remove(path); return {}; }
    auto ir = embx::ir::lower(*ast, err);
    if (!ir) { std::remove(path); return {}; }
    auto p = embx::plan::build(*ir, err);
    std::remove(path);
    return p;
}

TEST_CASE("layout graph", "[layout_graph]") {
    std::string err;
    auto p = makePlan("const N = 2; struct S { n: u8; items: u8[n]; }", err);
    REQUIRE((p && err.empty()));

    embx::plan::LayoutGraph g;
    REQUIRE(embx::plan::buildLayoutGraph(*p, g, err));
    REQUIRE(g.findNode("S", "n") != nullptr);
    REQUIRE(g.findNode("S", "items") != nullptr);
    REQUIRE(g.findNode("<module>", "N") != nullptr);
    REQUIRE(g.findNode(p->constants.front().symbol) != nullptr);
    const auto* n = static_cast<const embx::plan::Field*>(p->structs.front().members.front().get());
    REQUIRE(n != nullptr);
    REQUIRE(g.findNode(n->symbol) != nullptr);
    REQUIRE(!g.edges.empty());
    REQUIRE(g.findNode(p->constants.front().symbol) != nullptr);
    const auto* items = g.findNode("S", "items");
    REQUIRE(items != nullptr);
    REQUIRE(g.edges[0].from == items->id);
    REQUIRE(g.edges[0].to == n->symbol);
    REQUIRE(embx::plan::validateStaticLayoutSafety(*p, err));

    // A branch may depend on an earlier field, but the branch-local field must
    // not become a dependency of the enclosing scope.
    auto v = makePlan("struct S { tag: u8; variant v by tag { 1: { x: u8; y: u8[x]; } } z: u8; }", err);
    REQUIRE((v && err.empty()));
    embx::plan::LayoutGraph vg;
    REQUIRE(embx::plan::buildLayoutGraph(*v, vg, err));

    std::cout << "Layout graph tests passed\n";
}

TEST_CASE("layout graph omits compile-time constant dependency after materialization") {
    std::string err;
    auto p = makePlan("const N = 2; struct S { items: u8[N]; }", err);
    REQUIRE((p && err.empty()));
    embx::plan::LayoutGraph g;
    REQUIRE(embx::plan::buildLayoutGraph(*p, g, err));
    const auto* items = g.findNode("S", "items");
    REQUIRE(items != nullptr);
    // Statically evaluable dimensions are materialized in the execution
    // Plan. Therefore the compile-time constant N is no longer an execution
    // dependency of the field, and the layout graph must not contain a
    // runtime edge to N. The constant remains a declared graph node so its
    // canonical SymbolId is still represented by the graph.
    REQUIRE(g.findNode(p->constants.front().symbol) != nullptr);
    const auto constantSymbol = p->constants.front().symbol;
    bool hasConstantDependency = false;
    for (const auto& e : g.edges) {
        if (e.from == items->id && e.to == constantSymbol) {
            hasConstantDependency = true;
        }
    }
    REQUIRE_FALSE(hasConstantDependency);
}

TEST_CASE("layout graph resolves runtime parameter dependencies") {
    std::string err;
    auto p = makePlan("param N: u32; struct S { items: u8[N]; }", err);
    REQUIRE((p && err.empty()));

    embx::plan::LayoutGraph g;
    REQUIRE(embx::plan::buildLayoutGraph(*p, g, err));
    REQUIRE(p->parameters.size() == 1);
    const auto parameterSymbol = p->parameters.front().symbol;
    REQUIRE(g.findNode(parameterSymbol) != nullptr);

    const auto* items = g.findNode("S", "items");
    REQUIRE(items != nullptr);
    REQUIRE(std::any_of(g.edges.begin(), g.edges.end(), [&](const auto& e) {
        return e.from == items->id && e.to == parameterSymbol;
    }));
}

TEST_CASE("layout graph uses SymbolId for dependencies across scopes") {
    std::string err;
    auto p = makePlan("struct A { n: u8; xs: u8[n]; } struct B { n: u16; ys: u8[n]; }", err);
    REQUIRE((p && err.empty()));
    embx::plan::LayoutGraph g;
    REQUIRE(embx::plan::buildLayoutGraph(*p, g, err));

    const auto* aN = g.findNode("A", "n");
    const auto* aXs = g.findNode("A", "xs");
    const auto* bN = g.findNode("B", "n");
    const auto* bYs = g.findNode("B", "ys");
    REQUIRE(aN != nullptr);
    REQUIRE(aXs != nullptr);
    REQUIRE(bN != nullptr);
    REQUIRE(bYs != nullptr);
    REQUIRE(aN->symbol != bN->symbol);

    bool sawAXs = false;
    bool sawBYs = false;
    for (const auto& e : g.edges) {
        if (e.from == aXs->id) { REQUIRE(e.to == aN->symbol); sawAXs = true; }
        if (e.from == bYs->id) { REQUIRE(e.to == bN->symbol); sawBYs = true; }
    }
    REQUIRE(sawAXs);
    REQUIRE(sawBYs);
}

TEST_CASE("static layout safety traverses variants and nested layout operations") {
    embx::plan::Module module;
    embx::plan::Struct leaf; leaf.name = "Leaf";
    auto f = std::make_unique<embx::plan::Field>();
    f->name = "value"; f->type.kind = embx::core::TypeKind::Primitive; f->type.name = "u8";
    leaf.members.push_back(std::move(f));
    
    const auto leafId = embx::test::addStruct(module, std::move(leaf));

    embx::plan::Struct root; root.name = "Root";
    auto variant = std::make_unique<embx::plan::Variant>();
    variant->name = "payload";
    auto c = embx::plan::VariantCase{};
    c.type = embx::plan::Type{};
    c.type->kind = embx::core::TypeKind::Named; c.type->name = "Leaf"; c.type->reference.id = leafId;
    variant->cases.push_back(std::move(c));
    auto block = std::make_unique<embx::plan::Block>();
    block->name = "tail"; block->staticSize = 2;
    auto align = std::make_unique<embx::plan::Align>(); align->staticAlignment = 2;
    block->members.push_back(std::move(align));
    variant->defaultMembers.push_back(std::move(block));
    root.members.push_back(std::move(variant));
    
    embx::test::addStruct(module, std::move(root));

    std::string error;
    REQUIRE(embx::plan::validateStaticLayoutSafety(module, error));
    REQUIRE(error.empty());
}

TEST_CASE("static layout safety rejects zero static alignment") {
    embx::plan::Module module;
    embx::plan::Struct s; s.name = "Bad";
    auto a = std::make_unique<embx::plan::Align>();
    a->staticAlignment = 0;
    s.members.push_back(std::move(a));
    
    embx::test::addStruct(module, std::move(s));
    std::string error;
    REQUIRE_FALSE(embx::plan::validateStaticLayoutSafety(module, error));
    REQUIRE(error.find("alignment") != std::string::npos);
}
