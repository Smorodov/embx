#include <catch2/catch_test_macros.hpp>
#include "runtime/ExpressionEvaluator.h"
#include "core/Expr.h"
#include <iostream>
#include <limits>

TEST_CASE("expression", "[expression]") {
    using namespace embx;
    core::Expr lit; lit.kind=core::ExprKind::Literal; lit.text="42";
    runtime::Value v; std::string err; runtime::Environment env;
    REQUIRE(runtime::evaluate(&lit,env,v,err)); REQUIRE(std::get<int64_t>(v)==42);

    core::Expr truth; truth.kind=core::ExprKind::Literal; truth.text="true";
    REQUIRE(runtime::evaluate(&truth,env,v,err)); REQUIRE(std::get<bool>(v));
    core::Expr falsity; falsity.kind=core::ExprKind::Literal; falsity.text="false";
    REQUIRE(runtime::evaluate(&falsity,env,v,err)); REQUIRE(!std::get<bool>(v));

    core::Expr id; id.kind=core::ExprKind::Identifier; id.text="N"; env["N"]=uint64_t(7);
    runtime::LayoutSize n=0; REQUIRE(runtime::evaluateSize(&id,env,n,err)); REQUIRE(n==7);

    auto makeLit = [](const char* text){ auto e=std::make_unique<core::Expr>(); e->kind=core::ExprKind::Literal; e->text=text; return e; };
    auto left = makeLit("5");
    auto right = makeLit("3");
    core::Expr add; add.kind=core::ExprKind::Binary; add.op="+"; add.left=std::move(left); add.right=std::move(right);
    int64_t iv=0; REQUIRE(runtime::evaluateInteger(&add,env,iv,err)); REQUIRE(iv==8);

    auto five = makeLit("5");
    auto zero = makeLit("0");
    core::Expr div; div.kind=core::ExprKind::Binary; div.op="/"; div.left=std::move(five); div.right=std::move(zero);
    REQUIRE(!runtime::evaluate(&div,env,v,err)); REQUIRE(err=="division by zero");

    auto five2 = makeLit("5");
    core::Expr neg; neg.kind=core::ExprKind::Unary; neg.op="-"; neg.right=std::move(five2);
    runtime::LayoutSize sz=0; REQUIRE(runtime::evaluateSize(&neg,env,sz,err)==false); REQUIRE(err=="size/offset cannot be negative");

    auto big = makeLit("9223372036854775807");
    auto one = makeLit("1");
    core::Expr ov; ov.kind=core::ExprKind::Binary; ov.op="+"; ov.left=std::move(big); ov.right=std::move(one);
    REQUIRE(!runtime::evaluate(&ov,env,v,err)); REQUIRE(err=="integer overflow");

    env["U"] = uint64_t(0xFFFFFFFFFFFFFFFFULL);
    env["S"] = int64_t(-1);
    auto mixed = std::make_unique<core::Expr>(); mixed->kind=core::ExprKind::Binary; mixed->op="<";
    mixed->left=std::make_unique<core::Expr>(); mixed->left->kind=core::ExprKind::Identifier; mixed->left->text="S";
    mixed->right=std::make_unique<core::Expr>(); mixed->right->kind=core::ExprKind::Identifier; mixed->right->text="U";
    REQUIRE(!runtime::evaluate(mixed.get(), env, v, err)); REQUIRE(err=="mixed signed/unsigned operation with negative signed operand");

    auto badLogical = std::make_unique<core::Expr>();
    badLogical->kind = core::ExprKind::Binary; badLogical->op = "&&";
    badLogical->left = makeLit("0"); badLogical->right = makeLit("1");
    REQUIRE(!runtime::evaluate(badLogical.get(), env, v, err));
    REQUIRE(err == "logical operator requires boolean operands");

    std::cout<<"expression tests passed\n";
}
