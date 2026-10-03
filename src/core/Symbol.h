#pragma once
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace embx::core {

// Opaque declaration identity. Zero is reserved as invalid.
using SymbolId = std::uint64_t;
constexpr SymbolId InvalidSymbolId = 0;
// Built-in layout identity; it is not a declaration in SymbolTable.
constexpr SymbolId BuiltinNextSymbolId = std::numeric_limits<SymbolId>::max();
constexpr SymbolId BuiltinSizeInBytesSymbolId = BuiltinNextSymbolId - 1;
constexpr SymbolId BuiltinMinSizeInBytesSymbolId = BuiltinNextSymbolId - 2;
constexpr SymbolId BuiltinMaxSizeInBytesSymbolId = BuiltinNextSymbolId - 3;

enum class SymbolKind {
    TypeAlias,
    Struct,
    Enum,
    Constant,
    Callback,
    Parameter,
    EnumItem,
    AttributeDefinition,
    Field,
    VirtualField,
    AliasField
};

struct SymbolRef {
    SymbolId id = InvalidSymbolId;
    constexpr bool valid() const noexcept { return id != InvalidSymbolId; }
};

struct Symbol {
    SymbolId id = InvalidSymbolId;
    SymbolKind kind = SymbolKind::Struct;
    std::string name;
};

// Semantic name-resolution environment. It deliberately contains no AST or runtime state.
class Scope {
public:
    explicit Scope(const Scope* parent = nullptr) : parent_(parent) {}

    bool declare(const std::string& name, SymbolId id) {
        if (name.empty() || id == InvalidSymbolId || bindings_.count(name)) return false;
        bindings_.emplace(name, id);
        return true;
    }

    SymbolId resolve(const std::string& name) const noexcept {
        auto it = bindings_.find(name);
        if (it != bindings_.end()) return it->second;
        return parent_ ? parent_->resolve(name) : InvalidSymbolId;
    }

private:
    const Scope* parent_ = nullptr;
    std::unordered_map<std::string, SymbolId> bindings_;
};

class SymbolTable {
public:
    // Declare a module/global symbol. Its spelling participates in the
    // module-wide name index and therefore must be unique.
    SymbolId declare(SymbolKind kind, const std::string& name) {
        if (name.empty() || byName_.count(name)) return InvalidSymbolId;
        return append(kind, name, true);
    }

    // Declare a scope-owned symbol such as a field. Field spellings are only
    // unique within their Scope; the same spelling may legitimately occur in
    // multiple sibling scopes (or be shadowed in a child scope). Such symbols
    // intentionally do not enter the module-wide byName_ index.
    SymbolId declareScoped(SymbolKind kind, const std::string& name) {
        if (name.empty()) return InvalidSymbolId;
        return append(kind, name, false);
    }

    const Symbol* find(SymbolId id) const noexcept {
        if (id == InvalidSymbolId || id > symbols_.size()) return nullptr;
        return &symbols_[static_cast<std::size_t>(id - 1)];
    }

    SymbolId findId(const std::string& name) const noexcept {
        auto it = byName_.find(name);
        return it == byName_.end() ? InvalidSymbolId : it->second;
    }

    const std::vector<Symbol>& all() const noexcept { return symbols_; }

private:
    SymbolId append(SymbolKind kind, const std::string& name, bool indexByName) {
        const SymbolId id = static_cast<SymbolId>(symbols_.size() + 1);
        symbols_.push_back(Symbol{id, kind, name});
        if (indexByName) byName_.emplace(name, id);
        return id;
    }

    std::vector<Symbol> symbols_;
    std::unordered_map<std::string, SymbolId> byName_;
};

} // namespace embx::core
