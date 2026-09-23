#pragma once
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <string>
#include <unordered_set>
#include "core/Symbol.h"

namespace embx::core {

enum class ExprKind { Literal, Identifier, Unary, Binary, Parenthesized };

enum class ExprType {
    Unknown,
    IntegerSigned,
    IntegerUnsigned,
    Floating,
    Boolean,
    String,
    Invalid
};

inline const char* exprTypeName(ExprType type) noexcept {
    switch (type) {
    case ExprType::Unknown: return "unknown";
    case ExprType::IntegerSigned: return "signed integer";
    case ExprType::IntegerUnsigned: return "unsigned integer";
    case ExprType::Floating: return "floating";
    case ExprType::Boolean: return "boolean";
    case ExprType::String: return "string";
    case ExprType::Invalid: return "invalid";
    }
    return "invalid";
}

// Canonical, syntax-independent expression tree.  It deliberately has no
// dependency on the parser, AST, runtime, or any target-language backend.
struct Expr {
    ExprKind kind = ExprKind::Identifier;
    std::string text;
    std::string op;
    SymbolRef reference;
    ExprType type = ExprType::Unknown;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
};

inline ExprType inferLiteralExprType(const std::string& text, std::string& error) {
    if (text.empty()) { error = "empty literal"; return ExprType::Invalid; }
    if (text == "true" || text == "false") return ExprType::Boolean;
    if (text.front() == '"') return ExprType::String;
    if (text.find('.') != std::string::npos) {
        char* end = nullptr;
        errno = 0;
        const double value = std::strtod(text.c_str(), &end);
        if (errno == ERANGE || !end || *end || !std::isfinite(value)) {
            error = "invalid floating literal: " + text;
            return ExprType::Invalid;
        }
        return ExprType::Floating;
    }
    try {
        std::size_t index = 0;
        const bool hex = text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X');
        const auto value = std::stoull(text, &index, hex ? 16 : 10);
        if (index != text.size()) {
            error = "invalid integer literal: " + text;
            return ExprType::Invalid;
        }
        if (hex || value > static_cast<unsigned long long>(std::numeric_limits<std::int64_t>::max()))
            return ExprType::IntegerUnsigned;
        return ExprType::IntegerSigned;
    } catch (...) {
        error = "invalid integer literal: " + text;
        return ExprType::Invalid;
    }
}

inline ExprType inferExprType(const Expr* e, std::string& error) {
    if (!e) { error = "missing expression"; return ExprType::Invalid; }
    switch (e->kind) {
    case ExprKind::Literal:
        return inferLiteralExprType(e->text, error);
    case ExprKind::Identifier:
        if (e->text.empty()) { error = "empty expression node"; return ExprType::Invalid; }
        if (e->text == "$next" || e->text == "$size_in_bytes" || e->text == "$min_size_in_bytes" || e->text == "$max_size_in_bytes") return ExprType::IntegerUnsigned;
        return ExprType::Unknown;
    case ExprKind::Parenthesized:
        return inferExprType(e->left.get(), error);
    case ExprKind::Unary: {
        if (e->op != "-") { error = "unknown unary operator: " + e->op; return ExprType::Invalid; }
        const auto type = inferExprType(e->right.get(), error);
        if (type == ExprType::Invalid) return type;
        if (type == ExprType::Unknown) return type;
        if (type == ExprType::IntegerSigned || type == ExprType::IntegerUnsigned || type == ExprType::Floating)
            return type == ExprType::IntegerUnsigned ? ExprType::IntegerSigned : type;
        error = "unary '-' requires a numeric operand";
        return ExprType::Invalid;
    }
    case ExprKind::Binary: {
        static const std::unordered_set<std::string> arithmetic = {"+", "-", "*", "/", "%"};
        static const std::unordered_set<std::string> comparison = {"==", "!=", "<", "<=", ">", ">="};
        if (!arithmetic.count(e->op) && !comparison.count(e->op) && e->op != "&&" && e->op != "||") {
            error = "unknown operator: " + e->op;
            return ExprType::Invalid;
        }
        const auto left = inferExprType(e->left.get(), error);
        if (left == ExprType::Invalid) return left;
        const auto right = inferExprType(e->right.get(), error);
        if (right == ExprType::Invalid) return right;

        if (e->op == "&&" || e->op == "||") {
            if (left != ExprType::Boolean && left != ExprType::Unknown) {
                error = "logical operator requires boolean operands";
                return ExprType::Invalid;
            }
            if (right != ExprType::Boolean && right != ExprType::Unknown) {
                error = "logical operator requires boolean operands";
                return ExprType::Invalid;
            }
            return ExprType::Boolean;
        }

        if (comparison.count(e->op)) {
            const bool leftNumeric = left == ExprType::IntegerSigned || left == ExprType::IntegerUnsigned || left == ExprType::Floating;
            const bool rightNumeric = right == ExprType::IntegerSigned || right == ExprType::IntegerUnsigned || right == ExprType::Floating;
            if (left != ExprType::Unknown && right != ExprType::Unknown && !(leftNumeric && rightNumeric)) {
                error = "comparison operands must be numeric";
                return ExprType::Invalid;
            }
            return ExprType::Boolean;
        }

        if (e->op == "%" && ((left != ExprType::Unknown && left == ExprType::Floating) ||
                              (right != ExprType::Unknown && right == ExprType::Floating))) {
            error = "% requires integer operands";
            return ExprType::Invalid;
        }
        const bool leftNumeric = left == ExprType::IntegerSigned || left == ExprType::IntegerUnsigned || left == ExprType::Floating;
        const bool rightNumeric = right == ExprType::IntegerSigned || right == ExprType::IntegerUnsigned || right == ExprType::Floating;
        if ((left != ExprType::Unknown && !leftNumeric) || (right != ExprType::Unknown && !rightNumeric)) {
            error = "arithmetic operator requires numeric operands";
            return ExprType::Invalid;
        }
        if (left == ExprType::Floating || right == ExprType::Floating) return ExprType::Floating;
        if (left == ExprType::IntegerUnsigned || right == ExprType::IntegerUnsigned) return ExprType::IntegerUnsigned;
        if (left == ExprType::Unknown || right == ExprType::Unknown) return ExprType::Unknown;
        return ExprType::IntegerSigned;
    }
    }
    error = "unknown expression kind";
    return ExprType::Invalid;
}

inline bool annotateExprTypes(Expr* e, std::string& error) {
    if (!e) { error = "missing expression"; return false; }
    if (e->left && !annotateExprTypes(e->left.get(), error)) return false;
    if (e->right && !annotateExprTypes(e->right.get(), error)) return false;
    e->type = inferExprType(e, error);
    return e->type != ExprType::Invalid;
}

inline std::unique_ptr<Expr> cloneExpr(const Expr* e) {
    if (!e) return {};
    auto r = std::make_unique<Expr>();
    r->kind = e->kind;
    r->text = e->text;
    r->op = e->op;
    r->reference = e->reference;
    r->type = e->type;
    r->left = cloneExpr(e->left.get());
    r->right = cloneExpr(e->right.get());
    return r;
}

} // namespace embx::core
