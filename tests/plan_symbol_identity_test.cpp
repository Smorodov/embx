#include <catch2/catch_test_macros.hpp>
#include "plan/PlanBuilder.h"
#include "ir/Ir.h"

TEST_CASE("plan preserves canonical symbol identity") {
    embx::ir::Module ir;
    const auto sid = ir.symbolTable.declare(embx::core::SymbolKind::Struct, "Packet");
    auto st = std::make_unique<embx::ir::Struct>();
    st->symbol = sid;
    st->name = "Packet";
    ir.structs.push_back(std::move(st));
    ir.order.push_back({embx::ir::TopLevelRef::Kind::Struct, 0});

    std::string err;
    auto plan = embx::plan::build(ir, err);
    REQUIRE(plan);
    REQUIRE(err.empty());
    REQUIRE(plan->structs.size() == 1);
    REQUIRE(plan->structs[0].symbol == sid);
    REQUIRE(plan->structIndexBySymbol.at(sid) == 0);
}

TEST_CASE("plan rejects a declaration without SymbolId") {
    embx::ir::Module ir;
    auto st = std::make_unique<embx::ir::Struct>();
    st->name = "Broken";
    ir.structs.push_back(std::move(st));
    ir.order.push_back({embx::ir::TopLevelRef::Kind::Struct, 0});

    std::string err;
    auto plan = embx::plan::build(ir, err);
    REQUIRE_FALSE(plan);
    REQUIRE(err.find("invalid SymbolId") != std::string::npos);
}

TEST_CASE("plan rejects a declaration with the wrong SymbolKind") {
    embx::ir::Module ir;
    const auto sid = ir.symbolTable.declare(embx::core::SymbolKind::Enum, "Broken");
    auto st = std::make_unique<embx::ir::Struct>();
    st->symbol = sid;
    st->name = "Broken";
    ir.structs.push_back(std::move(st));
    ir.order.push_back({embx::ir::TopLevelRef::Kind::Struct, 0});

    std::string err;
    auto plan = embx::plan::build(ir, err);
    REQUIRE_FALSE(plan);
    REQUIRE(err.find("wrong kind") != std::string::npos);
}
