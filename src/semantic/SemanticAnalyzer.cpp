#include "semantic/SemanticAnalyzer.h"
#include <cctype>
#include <algorithm>
#include <functional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "core/Resolver.h"

namespace embx::semantic {
namespace {

using Symbols = std::set<std::string>;

bool primitive(const std::string& n) {
    static const Symbols p = {
        "u8","i8","u16","i16","u32","i32","u64","i64",
        "f32","f64","bytes","string"
    };
    return p.count(n) != 0;
}

bool integerPrimitive(const std::string& n) {
    static const Symbols p = {"u8","i8","u16","i16","u32","i32","u64","i64"};
    return p.count(n) != 0;
}

bool exprOk(const ast::Expr* e, std::string& err) {
    if (!e) { err = "missing expression"; return false; }
    return core::inferExprType(e, err) != core::ExprType::Invalid;
}



bool integerExpr(const ast::Expr* e, std::string& err) {
    if (!e) { err = "missing expression"; return false; }
    switch (e->kind) {
    case ast::ExprKind::Literal:
        if (e->text.empty() || e->text == "true" || e->text == "false" ||
            e->text.front() == '"' || e->text.find('.') != std::string::npos) {
            err = "integer expression required";
            return false;
        }
        return true;
    case ast::ExprKind::Identifier:
        if (e->text.empty()) { err = "empty expression node"; return false; }
        // Identifier type is resolved later; syntactically it is a valid
        // integer expression candidate (constants/parameters may supply it).
        return true;
    case ast::ExprKind::Parenthesized:
        return integerExpr(e->left.get(), err);
    case ast::ExprKind::Unary:
        if (e->op != "-") { err = "integer expression required"; return false; }
        return integerExpr(e->right.get(), err);
    case ast::ExprKind::Binary:
        if (e->op != "*" && e->op != "/" && e->op != "%" &&
            e->op != "+" && e->op != "-") {
            err = "integer expression required";
            return false;
        }
        return integerExpr(e->left.get(), err) && integerExpr(e->right.get(), err);
    }
    err = "integer expression required";
    return false;
}

bool enumExprInteger(const ast::Expr* e, std::string& err) {
    if (!e) { err = "missing expression"; return false; }
    switch (e->kind) {
    case ast::ExprKind::Literal:
        if (e->text.empty() || e->text == "true" || e->text == "false" ||
            e->text.front() == '"' || e->text.find('.') != std::string::npos) {
            err = "enum value must be an integer expression";
            return false;
        }
        return true;
    case ast::ExprKind::Identifier:
        return !e->text.empty(); // resolved later; constants/enum items are integer-valued
    case ast::ExprKind::Parenthesized:
        return enumExprInteger(e->left.get(), err);
    case ast::ExprKind::Unary:
        if (e->op != "-") { err = "enum value must be an integer expression"; return false; }
        return enumExprInteger(e->right.get(), err);
    case ast::ExprKind::Binary:
        if (e->op != "+" && e->op != "-" && e->op != "*" && e->op != "/" && e->op != "%") {
            err = "enum value must be an integer expression";
            return false;
        }
        return enumExprInteger(e->left.get(), err) && enumExprInteger(e->right.get(), err);
    }
    err = "enum value must be an integer expression";
    return false;
}


bool validateAttributes(const std::vector<ast::Attribute>& attrs,
                        const std::unordered_map<std::string, ast::AttributeType>& defs,
                        std::string& err) {
    std::set<std::string> seen;
    const auto isInteger = [](const std::string& v) {
        std::size_t i = v.rfind('-') == 0 ? 1 : 0;
        if (i == v.size()) return false;
        if (v.size() > i + 2 && v[i] == '0' && (v[i + 1] == 'x' || v[i + 1] == 'X')) {
            i += 2;
            if (i == v.size()) return false;
            for (; i < v.size(); ++i) if (!std::isxdigit(static_cast<unsigned char>(v[i]))) return false;
            return true;
        }
        for (; i < v.size(); ++i) if (!std::isdigit(static_cast<unsigned char>(v[i]))) return false;
        return true;
    };
    const auto isFloat = [](const std::string& v) {
        const std::string n = v.rfind('-') == 0 ? v.substr(1) : v;
        if (n.empty()) return false;
        bool dot = false, digit = false;
        for (char c : n) {
            if (c == '.') { if (dot) return false; dot = true; }
            else if (std::isdigit(static_cast<unsigned char>(c))) digit = true;
            else return false;
        }
        return dot && digit;
    };
    for (const auto& a : attrs) {
        if (!seen.insert(a.name).second) { err = "duplicate attribute: " + a.name; return false; }
        auto it = defs.find(a.name);
        if (it == defs.end()) { err = "unknown attribute: " + a.name; return false; }
        const auto expected = it->second;
        if (expected == ast::AttributeType::Marker) {
            if (a.hasValue) { err = "marker attribute cannot have a value: " + a.name; return false; }
            continue;
        }
        if (!a.hasValue) { err = "attribute requires a value: " + a.name; return false; }
        const std::string& v = a.value;
        const bool isString = v.size() >= 2 && v.front() == '"' && v.back() == '"';
        if (expected == ast::AttributeType::String && !isString) { err = "attribute requires string value: " + a.name; return false; }
        if (expected == ast::AttributeType::Float && !isFloat(v)) { err = "attribute requires floating-point value: " + a.name; return false; }
        if (expected == ast::AttributeType::Integer && !isInteger(v)) { err = "attribute requires integer value: " + a.name; return false; }
    }
    return true;
}

struct TypeResolver {
    const core::SymbolTable& symbols;
    const std::unordered_map<core::SymbolId, const ast::TypeAlias*>& aliases;
    core::NameResolver resolver;

    TypeResolver(const core::SymbolTable& table,
                 const std::unordered_map<core::SymbolId, const ast::TypeAlias*>& aliasMap)
        : symbols(table), aliases(aliasMap), resolver(table) {}

    core::SymbolId resolveId(const std::string& n) const noexcept {
        return resolver.resolve(n);
    }

    bool isTypeName(const std::string& n) const {
        if (primitive(n)) return true;
        const auto result = resolver.resolveDetailed(n);
        if (!result.success()) return false;
        const auto* s = symbols.find(result.id);
        return s && (s->kind == core::SymbolKind::TypeAlias ||
                     s->kind == core::SymbolKind::Struct ||
                     s->kind == core::SymbolKind::Enum);
    }

    bool resolveBase(const std::string& name, std::string& base,
                     std::vector<const ast::TypeSuffix*>& suffixes,
                     std::unordered_set<core::SymbolId>& visiting,
                     std::string& err) const {
        if (primitive(name)) {
            base = name;
            return true;
        }
        const auto id = resolveId(name);
        const auto* symbol = symbols.find(id);
        if (!symbol) { err = "unknown type: " + name; return false; }
        if (symbol->kind != core::SymbolKind::TypeAlias) {
            base = name;
            return true;
        }
        const auto it = aliases.find(id);
        if (it == aliases.end()) { err = "internal: missing alias declaration: " + name; return false; }
        if (!visiting.insert(id).second) {
            err = "cyclic type alias: " + name;
            return false;
        }
        const auto& target = it->second->target;
        for (const auto& s : target.suffixes) suffixes.push_back(&s);
        const bool ok = resolveBase(target.name, base, suffixes, visiting, err);
        visiting.erase(id);
        return ok;
    }

    bool resolve(const ast::TypeRef& t, std::string& base,
                 std::vector<const ast::TypeSuffix*>& suffixes,
                 std::string& err) const {
        if (t.name.empty()) { err = "empty type name"; return false; }
        if (!isTypeName(t.name)) { err = "unknown type: " + t.name; return false; }
        std::unordered_set<core::SymbolId> visiting;
        if (!resolveBase(t.name, base, suffixes, visiting, err)) return false;
        for (const auto& s : t.suffixes) suffixes.push_back(&s);
        return true;
    }

    bool resolveScalarInteger(const ast::TypeRef& t, std::string& err) const {
        std::string base;
        std::vector<const ast::TypeSuffix*> suffixes;
        if (!resolve(t, base, suffixes, err)) return false;
        if (!integerPrimitive(base)) {
            err = "enum underlying type must resolve to a scalar integer";
            return false;
        }
        if (!suffixes.empty()) {
            err = "enum underlying type must be a scalar integer";
            return false;
        }
        return true;
    }

    size_t resolvedSuffixCount(const ast::TypeRef& t, std::string& err) const {
        std::string base;
        std::vector<const ast::TypeSuffix*> suffixes;
        if (!resolve(t, base, suffixes, err)) return 0;
        return suffixes.size();
    }
};

bool containsNext(const ast::Expr* e) {
    if (!e) return false;
    if (e->kind == ast::ExprKind::Identifier && e->text == "$next") return true;
    return containsNext(e->left.get()) || containsNext(e->right.get());
}

bool rejectNextOutsideOffset(const ast::Expr* e, const char* context, std::string& err) {
    if (containsNext(e)) {
        err = std::string("$next is only valid in at(...) offset expressions: ") + context;
        return false;
    }
    return true;
}

bool typeShapeOk(const ast::TypeRef& t, const TypeResolver& resolver, std::string& err) {
    if (t.name.empty()) { err = "empty type name"; return false; }
    if (!resolver.isTypeName(t.name)) { err = "unknown type: " + t.name; return false; }

    // Validate the alias chain even when the current type has no suffix.
    std::string base;
    std::vector<const ast::TypeSuffix*> allSuffixes;
    if (!resolver.resolve(t, base, allSuffixes, err)) return false;

    if (!t.terminator.empty()) {
        if (base != "bytes") { err = "terminated sequence is only valid for bytes: " + t.name; return false; }
        if (t.terminator.size() > 0xFFFFFFFFu) { err = "terminated sequence terminator is too long"; return false; }
        if (!t.maxPayload.has_value()) { err = "terminated sequence requires a maximum payload length: " + t.name; return false; }
        if (!t.suffixes.empty()) { err = "terminated sequence cannot have an array suffix: " + t.name; return false; }
    }

    for (const auto* s : allSuffixes) {
        if (!s) { err = "null type suffix"; return false; }
        if (s->dynamic) {
            if (base != "bytes" && base != "string") {
                err = "[*] is only valid for bytes/string: " + t.name;
                return false;
            }
        } else if (containsNext(s->expr.get())) {
            err = "$next is only valid in at(...) offset expressions";
            return false;
        } else if (!integerExpr(s->expr.get(), err)) {
            err = "array length must be an integer expression";
            return false;
        }
    }
    return true;
}

bool members(const std::vector<std::unique_ptr<ast::Member>>& ms,
             const TypeResolver& resolver, const std::unordered_map<std::string, ast::AttributeType>& attributeDefs,
             const std::unordered_map<std::string, ast::CallbackDirection>& callbacks,
             std::string& err) {
    std::set<std::string> names;
    // Attribute declarations are resolved at module scope; attributes do not inherit.
    // Validate every member annotation locally.
    // (The declaration map is captured through the resolver only in the caller.)
    for (const auto& p : ms) {
        if (!p) { err = "null member"; return false; }
        if (!validateAttributes(p->attributes, attributeDefs, err)) return false;
        if (auto* f = dynamic_cast<const ast::Field*>(p.get())) {
            if (f->name.empty()) { err = "empty field name"; return false; }
            if (!names.insert(f->name).second) { err = "duplicate field: " + f->name; return false; }
            if (!typeShapeOk(f->type, resolver, err)) return false;
            if (f->bits != 0) { err = "bit width is only valid inside a bits block: " + f->name; return false; }
            size_t resolvedSuffixes = 0;
            {
                std::string typeErr;
                resolvedSuffixes = resolver.resolvedSuffixCount(f->type, typeErr);
                if (!typeErr.empty()) { err = typeErr; return false; }
            }
            bool hasLengthModifier = false;
            for (const auto& mod : f->modifiers) {
                if (mod.kind == ast::ModifierKind::Length) {
                    if (!mod.expr) { err = "missing length modifier expression: " + f->name; return false; }
                    if (!exprOk(mod.expr.get(), err)) return false;
                    if (!rejectNextOutsideOffset(mod.expr.get(), f->name.c_str(), err)) return false;
                    if (hasLengthModifier) { err = "duplicate length modifier: " + f->name; return false; }
                    hasLengthModifier = true;
                }
                if (mod.kind == ast::ModifierKind::BitWidth) { err = "bit width is only valid inside a bits block: " + f->name; return false; }
            }
            // A length belongs either to the type expression or to the field
            // modifier, but never to both.  This also catches an explicit
            // second suffix after an alias which already carries one:
            //   type B = bytes[4]; struct S { x: B[5]; }
            if (resolvedSuffixes > 1 || (resolvedSuffixes > 0 && hasLengthModifier)) {
                err = "array/length may be specified either in the type or as a field modifier, not both: " + f->name;
                return false;
            }
            if (f->transform) {
                if (f->transform->name != "scale") { err = "unknown transform: " + f->transform->name; return false; }
                if (f->transform->arguments.size() != 1) { err = "scale requires exactly one argument: " + f->name; return false; }
                std::string base; std::vector<const ast::TypeSuffix*> suffixes;
                if (!resolver.resolve(f->type, base, suffixes, err)) return false;
                if (!suffixes.empty() || !primitive(base) || base == "bytes" || base == "string") { err = "scale requires a scalar numeric wire type: " + f->name; return false; }
                if (!f->transform->arguments[0]) { err = "missing scale factor: " + f->name; return false; }
                const auto* a = f->transform->arguments[0].get();
                if (a->kind == ast::ExprKind::Identifier) {
                    const auto* sym = resolver.resolver.symbol(a->text);
                    if (!sym || sym->kind != core::SymbolKind::Constant) { err = "scale factor must be a compile-time constant: " + f->name; return false; }
                } else if (a->kind != ast::ExprKind::Literal && a->kind != ast::ExprKind::Parenthesized && a->kind != ast::ExprKind::Unary && a->kind != ast::ExprKind::Binary) {
                    err = "scale factor must be a compile-time constant: " + f->name; return false;
                }
                if (!exprOk(a, err)) return false;
                if (!rejectNextOutsideOffset(a, f->name.c_str(), err)) return false;
            }
        } else if (auto* v = dynamic_cast<const ast::Virtual*>(p.get())) {
            if (v->name.empty()) { err = "empty virtual field name"; return false; }
            if (!names.insert(v->name).second) { err = "duplicate member: " + v->name; return false; }
            if (!v->expression) { err = "missing virtual field expression: " + v->name; return false; }
            if (!exprOk(v->expression.get(), err)) return false;
            if (!rejectNextOutsideOffset(v->expression.get(), v->name.c_str(), err)) return false;
        } else if (auto* a = dynamic_cast<const ast::Alias*>(p.get())) {
            if (a->name.empty()) { err = "empty alias field name"; return false; }
            if (!names.insert(a->name).second) { err = "duplicate field: " + a->name; return false; }
            if (a->target.empty()) { err = "empty alias target: " + a->name; return false; }
        } else if (auto* bits = dynamic_cast<const ast::Bits*>(p.get())) {
            int total = 0;
            std::set<std::string> bitNames;
            for (const auto& bitField : bits->fields) {
                if (!bitNames.insert(bitField->name).second) { err = "duplicate bit field: " + bitField->name; return false; }
                if (bitField->bits < 1 || bitField->bits > 64) { err = "invalid bit width for field: " + bitField->name; return false; }
                if (!resolver.resolveScalarInteger(bitField->type, err)) {
                    if (err.rfind("unknown type:", 0) == 0) return false;
                    err = "bit field requires integer type (scalar integer required): " + bitField->name;
                    return false;
                }
                if (!bitField->type.suffixes.empty() || std::any_of(bitField->modifiers.begin(), bitField->modifiers.end(), [](const auto& m){ return m.kind == ast::ModifierKind::Length; })) { err = "bit field cannot be an array: " + bitField->name; return false; }
                if (!bitField->modifiers.empty()) { err = "bit field cannot have field modifiers: " + bitField->name; return false; }
                total += bitField->bits;
                if (total > 64) { err = "bits block exceeds 64 bits"; return false; }
            }
        } else if (auto* c = dynamic_cast<const ast::Conditional*>(p.get())) {
            if (!exprOk(c->condition.get(), err)) return false;
            if (!rejectNextOutsideOffset(c->condition.get(), "conditional condition", err)) return false;
            std::string conditionTypeError;
            const auto conditionType = core::inferExprType(c->condition.get(), conditionTypeError);
            if (conditionTypeError.empty() && conditionType != core::ExprType::Boolean) {
                err = "conditional condition must be boolean";
                return false;
            }
            if (!members(c->thenMembers, resolver, attributeDefs, callbacks, err)) return false;
            if (!members(c->elseMembers, resolver, attributeDefs, callbacks, err)) return false;
        } else if (auto* v = dynamic_cast<const ast::Variant*>(p.get())) {
            if (!exprOk(v->discriminator.get(), err)) return false;
            if (!rejectNextOutsideOffset(v->discriminator.get(), "variant discriminator", err)) return false;
            std::set<std::string> literalTags;
            for (const auto& c : v->cases) {
                if (!exprOk(c.tag.get(), err)) return false;
                if (!integerExpr(c.tag.get(), err)) { err = "variant case tag must be an integer expression"; return false; }
                if (!rejectNextOutsideOffset(c.tag.get(), "variant case tag", err)) return false;
                if (c.tag && c.tag->kind == ast::ExprKind::Literal) {
                    const auto tagType = core::inferLiteralExprType(c.tag->text, err);
                    if (tagType != core::ExprType::Invalid && tagType != core::ExprType::Floating &&
                        !literalTags.insert(c.tag->text).second) {
                        err = "duplicate variant tag";
                        return false;
                    }
                }
                if (c.type) {
                    if (!typeShapeOk(*c.type, resolver, err)) return false;
                } else if (!members(c.members, resolver, attributeDefs, callbacks, err)) return false;
            }
            if (v->defaultType) {
                if (!typeShapeOk(*v->defaultType, resolver, err)) return false;
            } else if (!v->defaultMembers.empty() && !members(v->defaultMembers, resolver, attributeDefs, callbacks, err)) return false;
        } else if (auto* block = dynamic_cast<const ast::Block*>(p.get())) {
            if (!rejectNextOutsideOffset(block->size.get(), "block size", err)) return false;
            if (!integerExpr(block->size.get(), err)) {
                err = "block size must be an integer expression";
                return false;
            }
            if (!members(block->members, resolver, attributeDefs, callbacks, err)) return false;
        } else if (auto* at = dynamic_cast<const ast::At*>(p.get())) {
            if (!integerExpr(at->offset.get(), err)) {
                err = "at offset must be an integer expression";
                return false;
            }
            if (!members(at->members, resolver, attributeDefs, callbacks, err)) return false;
        } else if (auto* align = dynamic_cast<const ast::Align*>(p.get())) {
            if (!rejectNextOutsideOffset(align->alignment.get(), "alignment", err)) return false;
            if (!integerExpr(align->alignment.get(), err)) {
                err = "alignment must be an integer expression";
                return false;
            }
        } else if (auto* c = dynamic_cast<const ast::Callback*>(p.get())) {
            if (c->direction == ast::CallbackDirection::Unspecified) {
                err = "callback name must start with on_decode or on_encode: " + c->name;
                return false;
            }
            const auto it = callbacks.find(c->name);
            if (it == callbacks.end()) {
                err = "callback is not declared: " + c->name;
                return false;
            }
            if (it->second != c->direction) {
                err = "callback direction does not match declaration: " + c->name;
                return false;
            }
            for (const auto& e : c->args) { if (!exprOk(e.get(), err)) return false; if (!rejectNextOutsideOffset(e.get(), "callback argument", err)) return false; }
        }
    }
    return true;
}

} // namespace

bool analyze(const ast::Module& m, std::string& err) {
    core::SymbolTable symbolTable;
    std::unordered_map<core::SymbolId, const ast::TypeAlias*> aliases;
    std::unordered_map<std::string, ast::AttributeType> attributeDefs;
    std::set<std::string> allSymbols;
    for (const auto& d : m.attributeDecls) {
        if (!attributeDefs.emplace(d.first, d.second).second) { err = "duplicate attribute declaration: " + d.first; return false; }
    }
    if (!validateAttributes(m.attributes, attributeDefs, err)) return false;

    auto addSymbol = [&](const std::string& name, core::SymbolKind kind) -> core::SymbolId {
        if (!allSymbols.insert(name).second) { err = "duplicate symbol: " + name; return core::InvalidSymbolId; }
        const auto id = symbolTable.declare(kind, name);
        if (id == core::InvalidSymbolId) { err = "duplicate symbol: " + name; return core::InvalidSymbolId; }
        return id;
    };

    for (const auto& a : m.aliases) {
        if (!validateAttributes(a.attributes, attributeDefs, err)) return false;
        const auto id = addSymbol(a.name, core::SymbolKind::TypeAlias);
        if (id == core::InvalidSymbolId) return false;
        aliases.emplace(id, &a);
    }
    for (const auto& s : m.structs) { if (!validateAttributes(s->attributes, attributeDefs, err)) return false; if (addSymbol(s->name, core::SymbolKind::Struct) == core::InvalidSymbolId) return false; }
    for (const auto& e : m.enums) { if (!validateAttributes(e.attributes, attributeDefs, err)) return false; if (addSymbol(e.name, core::SymbolKind::Enum) == core::InvalidSymbolId) return false; }
    for (const auto& c : m.constants) { if (!validateAttributes(c.attributes, attributeDefs, err)) return false; if (addSymbol(c.name, core::SymbolKind::Constant) == core::InvalidSymbolId) return false; }
    for (const auto& c : m.callbacks) {
        if (!validateAttributes(c.attributes, attributeDefs, err)) return false;
        if (c.direction == ast::CallbackDirection::Unspecified) {
            err = "callback name must start with on_decode or on_encode: " + c.name;
            return false;
        }
        if (addSymbol(c.name, core::SymbolKind::Callback) == core::InvalidSymbolId) return false;
    }
    for (const auto& p : m.parameters) {
        if (!validateAttributes(p.attributes, attributeDefs, err)) return false;
        if (addSymbol(p.name, core::SymbolKind::Parameter) == core::InvalidSymbolId) return false;
        if (!primitive(p.type.name) || p.type.name == "bytes" || p.type.name == "string" || !p.type.suffixes.empty()) {
            err = "runtime parameter must be a scalar primitive type: " + p.name;
            return false;
        }
    }

    std::unordered_map<std::string, ast::CallbackDirection> callbackDirections;
    for (const auto& c : m.callbacks) callbackDirections.emplace(c.name, c.direction);

    TypeResolver resolver{symbolTable, aliases};

    // Resolve every alias independently of declaration order. This makes the
    // alias graph a first-class semantic contract and produces deterministic
    // cycle diagnostics before later member/type checks run.
    std::function<bool(const std::string&, std::unordered_set<core::SymbolId>&)> aliasDfs;
    aliasDfs = [&](const std::string& name, std::unordered_set<core::SymbolId>& visiting) -> bool {
        const auto id = symbolTable.findId(name);
        const auto it = aliases.find(id);
        if (it == aliases.end()) return true;
        if (!visiting.insert(id).second) { err = "cyclic type alias: " + name; return false; }
        if (!typeShapeOk(it->second->target, resolver, err)) return false;
        const std::string target = it->second->target.name;
        if (symbolTable.findId(target) != core::InvalidSymbolId && symbolTable.find(symbolTable.findId(target))->kind == core::SymbolKind::TypeAlias && !aliasDfs(target, visiting)) return false;
        visiting.erase(id);
        return true;
    };
    for (const auto& a : m.aliases) {
        std::unordered_set<core::SymbolId> visiting;
        if (!aliasDfs(a.name, visiting)) return false;
    }

    for (const auto& e : m.enums) {
        if (e.underlying) {
            if (!typeShapeOk(*e.underlying, resolver, err)) return false;
            if (!resolver.resolveScalarInteger(*e.underlying, err)) {
                return false;
            }
        }
        std::set<std::string> items;
        for (const auto& i : e.items) {
            if (!items.insert(i.name).second) { err = "duplicate enum item: " + i.name; return false; }
            if (!exprOk(i.value.get(), err)) return false;
            if (!rejectNextOutsideOffset(i.value.get(), i.name.c_str(), err)) return false;
            if (!enumExprInteger(i.value.get(), err)) {
                err = "enum " + e.name + " item " + i.name + ": " + err;
                return false;
            }
        }
    }

    for (const auto& c : m.constants) { if (!exprOk(c.expr.get(), err)) return false; if (!rejectNextOutsideOffset(c.expr.get(), c.name.c_str(), err)) return false; }
    for (const auto& s : m.structs) {
        if (!members(s->members, resolver, attributeDefs, callbackDirections, err)) {
            err = "in struct " + s->name + ": " + err;
            return false;
        }
    }
    return true;
}

} // namespace embx::semantic
