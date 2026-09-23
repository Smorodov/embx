#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <limits>
#include "runtime/ExpressionEvaluator.h"
#include "core/Expr.h"

TEST_CASE("layout size uses the full 64-bit logical domain") {
    embx::runtime::Environment env;
    env["MAX"] = std::numeric_limits<std::uint64_t>::max();
    embx::core::Expr id; id.kind = embx::core::ExprKind::Identifier; id.text = "MAX";
    embx::runtime::LayoutSize n = 0;
    std::string err;
    REQUIRE(embx::runtime::evaluateSize(&id, env, n, err));
    REQUIRE(n == std::numeric_limits<std::uint64_t>::max());
}

TEST_CASE("negative layout size remains invalid") {
    embx::runtime::Environment env;
    env["N"] = std::int64_t(-1);
    embx::core::Expr id; id.kind = embx::core::ExprKind::Identifier; id.text = "N";
    embx::runtime::LayoutSize n = 0;
    std::string err;
    REQUIRE_FALSE(embx::runtime::evaluateSize(&id, env, n, err));
    REQUIRE(err == "size/offset cannot be negative");
}

TEST_CASE("overflow is rejected in the logical 64-bit domain") {
    embx::runtime::Environment env;
    env["MAX"] = std::numeric_limits<std::uint64_t>::max();
    embx::core::Expr left; left.kind = embx::core::ExprKind::Identifier; left.text = "MAX";
    embx::core::Expr right; right.kind = embx::core::ExprKind::Literal; right.text = "1";
    embx::core::Expr add; add.kind = embx::core::ExprKind::Binary; add.op = "+";
    add.left = std::make_unique<embx::core::Expr>();
    add.left->kind = left.kind;
    add.left->text = left.text;
    add.right = std::make_unique<embx::core::Expr>();
    add.right->kind = right.kind;
    add.right->text = right.text;
    embx::runtime::LayoutSize n = 0;
    std::string err;
    REQUIRE_FALSE(embx::runtime::evaluateSize(&add, env, n, err));
    REQUIRE(err == "integer overflow");
}
