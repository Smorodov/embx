#pragma once
#include "core/Expr.h"
#include "core/Symbol.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>

namespace embx::runtime {

using Value = std::variant<int64_t, uint64_t, double, bool>;
using LayoutSize = std::uint64_t;
using Environment = std::unordered_map<std::string, Value>;
using SymbolEnvironment = std::unordered_map<core::SymbolId, Value>;

bool evaluate(const core::Expr* expr, const Environment& env, Value& out, std::string& error);
bool evaluate(const core::Expr* expr, const SymbolEnvironment& env, Value& out, std::string& error);
bool evaluateInteger(const core::Expr* expr, const Environment& env, int64_t& out, std::string& error);
bool evaluateInteger(const core::Expr* expr, const SymbolEnvironment& env, int64_t& out, std::string& error);
bool evaluateSize(const core::Expr* expr, const Environment& env, LayoutSize& out, std::string& error);
bool evaluateSize(const core::Expr* expr, const SymbolEnvironment& env, LayoutSize& out, std::string& error);

} // namespace embx::runtime
