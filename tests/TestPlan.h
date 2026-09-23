#pragma once
#include "plan/Plan.h"
#include <string>
#include <stdexcept>
#include <utility>

namespace embx::test {

inline void addFieldSymbols(plan::Module& m, std::vector<std::unique_ptr<plan::Op>>& members) {
    for (auto& op : members) {
        if (!op) continue;
        switch (op->kind) {
        case plan::OpKind::Field: {
            auto& f = static_cast<plan::Field&>(*op);
            if (f.symbol == core::InvalidSymbolId) f.symbol = m.symbolTable.declareScoped(core::SymbolKind::Field, f.name);
            break;
        }
        case plan::OpKind::Bits: {
            auto& b = static_cast<plan::Bits&>(*op);
            for (auto& f : b.fields)
                if (f.symbol == core::InvalidSymbolId) f.symbol = m.symbolTable.declareScoped(core::SymbolKind::Field, f.name);
            break;
        }
        case plan::OpKind::Conditional: {
            auto& c = static_cast<plan::Conditional&>(*op);
            addFieldSymbols(m, c.thenMembers);
            addFieldSymbols(m, c.elseMembers);
            break;
        }
        case plan::OpKind::Variant: {
            auto& v = static_cast<plan::Variant&>(*op);
            for (auto& c : v.cases) addFieldSymbols(m, c.members);
            addFieldSymbols(m, v.defaultMembers);
            break;
        }
        case plan::OpKind::Block: addFieldSymbols(m, static_cast<plan::Block&>(*op).members); break;
        case plan::OpKind::At: addFieldSymbols(m, static_cast<plan::At&>(*op).members); break;
        default: break;
        }
    }
}

inline core::SymbolId addStruct(plan::Module& m, plan::Struct s) {
    addFieldSymbols(m, s.members);
    const auto id = m.symbolTable.declare(core::SymbolKind::Struct, s.name);
    if (id == core::InvalidSymbolId) throw std::runtime_error("duplicate test struct symbol: " + s.name);
    s.symbol = id;
    const auto index = m.structs.size();
    m.structIndexBySymbol[id] = index;
    m.structs.push_back(std::move(s));
    return id;
}

inline core::SymbolId addAlias(plan::Module& m, plan::Alias a) {
    const auto id = m.symbolTable.declare(core::SymbolKind::TypeAlias, a.name);
    if (id == core::InvalidSymbolId) throw std::runtime_error("duplicate test alias symbol: " + a.name);
    a.symbol = id;
    const auto index = m.aliases.size();
    m.aliasIndexBySymbol[id] = index;
    m.aliases.push_back(std::move(a));
    return id;
}

inline core::SymbolId addEnum(plan::Module& m, plan::Enum e) {
    const auto id = m.symbolTable.declare(core::SymbolKind::Enum, e.name);
    if (id == core::InvalidSymbolId) throw std::runtime_error("duplicate test enum symbol: " + e.name);
    e.symbol = id;
    const auto index = m.enums.size();
    m.enumIndexBySymbol[id] = index;
    m.enums.push_back(std::move(e));
    return id;
}

inline core::SymbolId addConstant(plan::Module& m, plan::Constant c) {
    const auto id = m.symbolTable.declare(core::SymbolKind::Constant, c.name);
    if (id == core::InvalidSymbolId) throw std::runtime_error("duplicate test constant symbol: " + c.name);
    c.symbol = id;
    const auto index = m.constants.size();
    m.constantIndexBySymbol[id] = index;
    m.constants.push_back(std::move(c));
    return id;
}

inline core::SymbolId addFieldSymbol(plan::Module& m, plan::Field& f) {
    const auto id = m.symbolTable.declareScoped(core::SymbolKind::Field, f.name);
    if (id == core::InvalidSymbolId) throw std::runtime_error("invalid test field symbol: " + f.name);
    f.symbol = id;
    return id;
}

inline void bind(plan::Type& t, core::SymbolId id) { t.reference.id = id; }
inline void bind(plan::Expr& e, core::SymbolId id) { e.reference.id = id; }

} // namespace embx::test
