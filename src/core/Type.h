#pragma once
#include "core/Expr.h"
#include "core/Symbol.h"
#include <cstdint>
#include <memory>
#include <string>
#include <optional>
#include <utility>
#include <vector>

namespace embx::core {

enum class TypeKind { Primitive, Bytes, String, Named };

// One semantic dimension.  Fixed carries its canonical expression; Remaining
// represents the language's [*] extent.  No AST or target-language objects are
// retained here.
struct Dimension {
    enum class Kind { Fixed, Dynamic, Remaining };
    Kind kind = Kind::Fixed;
    std::unique_ptr<Expr> expression;

    static Dimension fixed(std::unique_ptr<Expr> e) {
        Dimension d; d.kind = Kind::Fixed; d.expression = std::move(e); return d;
    }
    static Dimension dynamic(std::unique_ptr<Expr> e) {
        Dimension d; d.kind = Kind::Dynamic; d.expression = std::move(e); return d;
    }
    static Dimension remaining() {
        Dimension d; d.kind = Kind::Remaining; return d;
    }

    Dimension() = default;
    Dimension(const Dimension& other) : kind(other.kind), expression(cloneExpr(other.expression.get())) {}
    Dimension& operator=(const Dimension& other) {
        if (this != &other) { kind = other.kind; expression = cloneExpr(other.expression.get()); }
        return *this;
    }
    Dimension(Dimension&&) noexcept = default;
    Dimension& operator=(Dimension&&) noexcept = default;
};

// Canonical semantic type.  It deliberately has no dependency on AST,
// parser, runtime, layout, or any target-language backend.
struct Type {
    TypeKind kind = TypeKind::Named;
    std::string name;
    SymbolRef reference;
    std::vector<Dimension> dimensions;
    std::vector<std::uint8_t> terminator;
    std::optional<std::uint64_t> maxPayload;

    Type() = default;
    Type(const Type& other) : kind(other.kind), name(other.name), reference(other.reference), dimensions(other.dimensions), terminator(other.terminator), maxPayload(other.maxPayload) {}
    Type& operator=(const Type& other) {
        if (this != &other) { kind = other.kind; name = other.name; reference = other.reference; dimensions = other.dimensions; terminator = other.terminator; maxPayload = other.maxPayload; }
        return *this;
    }
    Type(Type&&) noexcept = default;
    Type& operator=(Type&&) noexcept = default;
};

inline bool hasRemaining(const Type& t) {
    for (const auto& d : t.dimensions) if (d.kind == Dimension::Kind::Remaining) return true;
    return false;
}

} // namespace embx::core
