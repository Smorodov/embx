#include <catch2/catch_test_macros.hpp>
#include "core/Type.h"
#include <memory>

namespace {
std::unique_ptr<embx::core::Expr> literal(const char* text) {
    auto e = std::make_unique<embx::core::Expr>();
    e->kind = embx::core::ExprKind::Literal;
    e->text = text;
    return e;
}
}

TEST_CASE("canonical type is self contained", "[core][type]") {
    using namespace embx::core;
    Type t;
    t.kind = TypeKind::Primitive;
    t.name = "u16";
    t.dimensions.push_back(Dimension::fixed(literal("4")));
    t.dimensions.push_back(Dimension::remaining());

    REQUIRE(t.kind == TypeKind::Primitive);
    REQUIRE(t.name == "u16");
    REQUIRE(t.dimensions.size() == 2);
    REQUIRE(t.dimensions[0].kind == Dimension::Kind::Fixed);
    REQUIRE(t.dimensions[0].expression);
    REQUIRE(t.dimensions[0].expression->text == "4");
    REQUIRE(t.dimensions[1].kind == Dimension::Kind::Remaining);
    REQUIRE(hasRemaining(t));
}

TEST_CASE("canonical type deep copies dimensions and expressions", "[core][type]") {
    using namespace embx::core;
    Type a;
    a.kind = TypeKind::Named;
    a.name = "Packet";
    a.dimensions.push_back(Dimension::fixed(literal("N + 1")));

    Type b = a;
    REQUIRE(b.name == "Packet");
    REQUIRE(b.dimensions.size() == 1);
    REQUIRE(b.dimensions[0].expression);
    REQUIRE(b.dimensions[0].expression != a.dimensions[0].expression);
    REQUIRE(b.dimensions[0].expression->text == "N + 1");

    b.dimensions[0].expression->text = "7";
    REQUIRE(a.dimensions[0].expression->text == "N + 1");
}

TEST_CASE("remaining extent is semantic, not an AST pointer", "[core][type]") {
    using namespace embx::core;
    Type t;
    t.kind = TypeKind::Bytes;
    t.name = "bytes";
    t.dimensions.push_back(Dimension::remaining());

    REQUIRE(hasRemaining(t));
    REQUIRE_FALSE(t.dimensions[0].expression);
}
