#include "core/Expr.h"
#include "runtime/ExpressionEvaluator.h"
#include <cassert>
#include <iostream>

using embx::core::Expr;
using embx::core::ExprKind;

static std::unique_ptr<Expr> lit(const char* text) {
    auto e = std::make_unique<Expr>(); e->kind=ExprKind::Literal; e->text=text; return e;
}
static std::unique_ptr<Expr> id(const char* text) {
    auto e = std::make_unique<Expr>(); e->kind=ExprKind::Identifier; e->text=text; return e;
}
static std::unique_ptr<Expr> bin(const char* op, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r) {
    auto e = std::make_unique<Expr>(); e->kind=ExprKind::Binary; e->op=op; e->left=std::move(l); e->right=std::move(r); return e;
}

int main() {
    embx::runtime::Environment env;
    embx::runtime::Value out;
    std::string error;

    auto andExpr = bin("&&", bin("==", lit("0"), lit("1")), id("missing"));
    assert(embx::runtime::evaluate(andExpr.get(), env, out, error));
    assert(std::get<bool>(out) == false);
    assert(error.empty());

    auto orExpr = bin("||", bin("==", lit("1"), lit("1")), id("missing"));
    assert(embx::runtime::evaluate(orExpr.get(), env, out, error));
    assert(std::get<bool>(out) == true);
    assert(error.empty());

    std::cout << "core_expression_test passed\n";
}
