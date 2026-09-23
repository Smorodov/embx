#include <catch2/catch_test_macros.hpp>
#include "core/Expr.h"
#include "core/Type.h"
#include "core/Resolver.h"
#include "runtime/ExpressionEvaluator.h"

TEST_CASE("resolved expressions evaluate through SymbolId") {
    embx::core::Expr e;
    e.kind = embx::core::ExprKind::Identifier;
    e.text = "N";
    e.reference.id = 42;
    embx::runtime::SymbolEnvironment env;
    env.emplace(42, int64_t(7));
    embx::runtime::Value out; std::string err;
    REQUIRE(embx::runtime::evaluate(&e, env, out, err));
    REQUIRE(std::get<int64_t>(out) == 7);
}

TEST_CASE("unresolved identifiers are rejected by SymbolId evaluator") {
    embx::core::Expr e;
    e.kind = embx::core::ExprKind::Identifier;
    e.text = "N";
    embx::runtime::SymbolEnvironment env;
    embx::runtime::Value out; std::string err;
    REQUIRE_FALSE(embx::runtime::evaluate(&e, env, out, err));
    REQUIRE(err.find("unresolved identifier") != std::string::npos);
}

TEST_CASE("NameResolver is the canonical local-over-parent symbol lookup") {
    embx::core::SymbolTable symbols;
    const auto typeId = symbols.declare(embx::core::SymbolKind::Struct, "Packet");
    const auto parentId = symbols.declareScoped(embx::core::SymbolKind::Field, "value");
    const auto childId = symbols.declareScoped(embx::core::SymbolKind::Field, "value2");
    REQUIRE(typeId != embx::core::InvalidSymbolId);
    REQUIRE(parentId != embx::core::InvalidSymbolId);
    REQUIRE(childId != embx::core::InvalidSymbolId);

    embx::core::Scope parent;
    REQUIRE(parent.declare("value", parentId));
    embx::core::Scope child(&parent);
    REQUIRE(child.declare("value", childId));

    embx::core::NameResolver resolver(symbols, &child);
    REQUIRE(resolver.resolve("value") == childId);
    REQUIRE(resolver.resolve("Packet") == typeId);
    REQUIRE(resolver.resolve("missing") == embx::core::InvalidSymbolId);
}

TEST_CASE("NameResolver exposes explicit resolution status") {
    embx::core::SymbolTable symbols;
    const auto packet = symbols.declare(embx::core::SymbolKind::Struct, "Packet");
    REQUIRE(embx::core::NameResolver(symbols).resolveDetailed("Packet").status == embx::core::ResolveStatus::Resolved);
    REQUIRE(embx::core::NameResolver(symbols).resolveDetailed("").status == embx::core::ResolveStatus::EmptyName);
    REQUIRE(embx::core::NameResolver(symbols).resolveDetailed("Missing").status == embx::core::ResolveStatus::Unresolved);
    REQUIRE(embx::core::NameResolver(symbols).resolveQualified("Packet").id == packet);
    REQUIRE(embx::core::NameResolver(symbols).resolveKind("Packet", embx::core::SymbolKind::Struct).id == packet);
    REQUIRE(embx::core::NameResolver(symbols).resolveKind("Packet", embx::core::SymbolKind::Enum).status == embx::core::ResolveStatus::KindMismatch);
}

TEST_CASE("NameResolver kind checks distinguish non-types from types") {
    embx::core::SymbolTable symbols;
    const auto constant = symbols.declare(embx::core::SymbolKind::Constant, "N");
    const auto packet = symbols.declare(embx::core::SymbolKind::Struct, "Packet");
    REQUIRE(constant != embx::core::InvalidSymbolId);
    REQUIRE(packet != embx::core::InvalidSymbolId);
    REQUIRE(embx::core::NameResolver(symbols).resolveKind("N", embx::core::SymbolKind::Struct).status == embx::core::ResolveStatus::KindMismatch);
    REQUIRE(embx::core::NameResolver(symbols).resolveKind("Packet", embx::core::SymbolKind::Struct).success());
}
