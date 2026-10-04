#include <catch2/catch_test_macros.hpp>
#include "core/Expr.h"
#include <memory>
#include <string>

namespace {
std::unique_ptr<embx::core::Expr> lit(const char* text) {
    auto e = std::make_unique<embx::core::Expr>();
    e->kind = embx::core::ExprKind::Literal;
    e->text = text;
    return e;
}
std::unique_ptr<embx::core::Expr> bin(const char* op,
                                     std::unique_ptr<embx::core::Expr> left,
                                     std::unique_ptr<embx::core::Expr> right) {
    auto e = std::make_unique<embx::core::Expr>();
    e->kind = embx::core::ExprKind::Binary;
    e->op = op;
    e->left = std::move(left);
    e->right = std::move(right);
    return e;
}
}

TEST_CASE("canonical expression type rules", "[expression_type]") {
    using embx::core::ExprType;
    std::string error;

    auto a = lit("42");
    REQUIRE(embx::core::inferExprType(a.get(), error) == ExprType::IntegerSigned);
    REQUIRE(error.empty());

    auto h = lit("0xFFFFFFFFFFFFFFFF");
    REQUIRE(embx::core::inferExprType(h.get(), error) == ExprType::IntegerUnsigned);
    REQUIRE(error.empty());

    auto f = lit("1.5");
    REQUIRE(embx::core::inferExprType(f.get(), error) == ExprType::Floating);

    auto s = lit("\"x\"");
    REQUIRE(embx::core::inferExprType(s.get(), error) == ExprType::String);

    auto t = lit("true");
    REQUIRE(embx::core::inferExprType(t.get(), error) == ExprType::Boolean);
    REQUIRE(error.empty());
    auto f0 = lit("false");
    REQUIRE(embx::core::inferExprType(f0.get(), error) == ExprType::Boolean);
    REQUIRE(error.empty());

    auto cmp = bin("<", lit("1"), lit("2"));
    REQUIRE(embx::core::inferExprType(cmp.get(), error) == ExprType::Boolean);
    REQUIRE(error.empty());

    auto logical = bin("&&", std::move(cmp), bin("==", lit("2"), lit("2")));
    REQUIRE(embx::core::inferExprType(logical.get(), error) == ExprType::Boolean);
    REQUIRE(error.empty());

    auto badLogical = bin("&&", lit("1"), lit("2"));
    REQUIRE(embx::core::inferExprType(badLogical.get(), error) == ExprType::Invalid);
    REQUIRE(error == "logical operator requires boolean operands");

    auto badModulo = bin("%", lit("1.0"), lit("2"));
    REQUIRE(embx::core::inferExprType(badModulo.get(), error) == ExprType::Invalid);
    REQUIRE(error == "% requires integer operands");

    auto badArithmetic = bin("+", lit("\"x\""), lit("1"));
    REQUIRE(embx::core::inferExprType(badArithmetic.get(), error) == ExprType::Invalid);
    REQUIRE(error == "arithmetic operator requires numeric operands");
}
