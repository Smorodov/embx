#pragma once

#include "core/Symbol.h"
#include <string>

namespace embx::core {

// Canonical semantic name resolver used between AST lowering and later phases.
// It deliberately knows only about symbols/scopes; it does not know about AST,
// parser, IR, Plan or a backend.
enum class ResolveStatus { Resolved, EmptyName, Unresolved, KindMismatch };

struct ResolveResult {
    SymbolId id = InvalidSymbolId;
    ResolveStatus status = ResolveStatus::Unresolved;
    constexpr bool success() const noexcept { return status == ResolveStatus::Resolved; }
};

class NameResolver {
public:
    NameResolver(const SymbolTable& symbols, const Scope* scope = nullptr)
        : symbols_(symbols), scope_(scope) {}

    ResolveResult resolveDetailed(const std::string& name) const noexcept {
        if (name.empty()) return {InvalidSymbolId, ResolveStatus::EmptyName};
        if (scope_) {
            const auto local = scope_->resolve(name);
            if (local != InvalidSymbolId) return {local, ResolveStatus::Resolved};
        }
        const auto global = symbols_.findId(name);
        return global == InvalidSymbolId
            ? ResolveResult{InvalidSymbolId, ResolveStatus::Unresolved}
            : ResolveResult{global, ResolveStatus::Resolved};
    }

    SymbolId resolve(const std::string& name) const noexcept {
        return resolveDetailed(name).id;
    }

    ResolveResult resolveQualified(const std::string& name) const noexcept {
        // Qualified names are canonical module symbols. They must not be
        // accidentally captured by a local field with the same spelling.
        if (name.empty()) return {InvalidSymbolId, ResolveStatus::EmptyName};
        const auto id = symbols_.findId(name);
        return id == InvalidSymbolId
            ? ResolveResult{InvalidSymbolId, ResolveStatus::Unresolved}
            : ResolveResult{id, ResolveStatus::Resolved};
    }

    ResolveResult resolveKind(const std::string& name, SymbolKind expected) const noexcept {
        const auto result = resolveDetailed(name);
        if (!result.success()) return result;
        const auto* s = symbols_.find(result.id);
        if (!s || s->kind != expected)
            return {InvalidSymbolId, ResolveStatus::KindMismatch};
        return result;
    }

    bool resolveRequired(const std::string& name, SymbolId& out, std::string& error) const {
        out = resolve(name);
        if (out != InvalidSymbolId) return true;
        error = "unresolved name: " + name;
        return false;
    }

    const Symbol* symbol(const std::string& name) const noexcept {
        return symbols_.find(resolve(name));
    }

    const Scope* scope() const noexcept { return scope_; }
    const SymbolTable& symbols() const noexcept { return symbols_; }

private:
    const SymbolTable& symbols_;
    const Scope* scope_ = nullptr;
};

} // namespace embx::core
