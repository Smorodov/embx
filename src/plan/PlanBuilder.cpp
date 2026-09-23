#include "plan/PlanBuilder.h"
#include <cmath>
#include "plan/LayoutGraph.h"
#include "core/Semantic.h"
#include <functional>
#include <algorithm>
#include <limits>
#include <set>
#include <unordered_set>

namespace embx::plan {
namespace {

bool evaluateExpr(const core::Expr* e, const runtime::SymbolEnvironment& env, runtime::Value& out, std::string& err) {
    return runtime::evaluate(e, env, out, err);
}

bool evaluateExprSize(const core::Expr* e, const runtime::SymbolEnvironment& env, runtime::LayoutSize& out, std::string& err) {
    return runtime::evaluateSize(e, env, out, err);
}

core::ExprType expressionTypeFromType(const ir::Module& src, const core::Type& t,
                                      std::unordered_set<core::SymbolId>& visiting) {
    if (!t.dimensions.empty()) return core::ExprType::Invalid;
    if (t.kind == core::TypeKind::Bytes || t.kind == core::TypeKind::String) return core::ExprType::Invalid;
    if (t.kind == core::TypeKind::Primitive) {
        if (t.name == "u8" || t.name == "u16" || t.name == "u32" || t.name == "u64") return core::ExprType::IntegerUnsigned;
        if (t.name == "i8" || t.name == "i16" || t.name == "i32" || t.name == "i64") return core::ExprType::IntegerSigned;
        if (t.name == "f32" || t.name == "f64") return core::ExprType::Floating;
        return core::ExprType::Invalid;
    }
    if (t.kind != core::TypeKind::Named || !t.reference.valid()) return core::ExprType::Invalid;
    const auto* sym = src.symbolTable.find(t.reference.id);
    if (!sym || !visiting.insert(t.reference.id).second) return core::ExprType::Invalid;
    core::ExprType result = core::ExprType::Invalid;
    if (sym->kind == core::SymbolKind::TypeAlias) {
        for (const auto& a : src.aliases) {
            if (a.symbol == t.reference.id) {
                result = expressionTypeFromType(src, a.target, visiting);
                break;
            }
        }
    } else if (sym->kind == core::SymbolKind::Enum) {
        for (const auto& e : src.enums) {
            if (e.symbol == t.reference.id) {
                if (e.underlying.name.empty()) result = core::ExprType::IntegerUnsigned;
                else result = expressionTypeFromType(src, e.underlying, visiting);
                break;
            }
        }
    }
    visiting.erase(t.reference.id);
    return result;
}

core::ExprType expressionTypeFromMembers(const ir::Module& src,
                                          const std::vector<std::unique_ptr<ir::Member>>& members,
                                          core::SymbolId id) {
    for (const auto& m : members) {
        if (!m) continue;
        if (const auto* f = dynamic_cast<const ir::Field*>(m.get()); f && f->symbol == id) {
            std::unordered_set<core::SymbolId> visiting;
            return expressionTypeFromType(src, f->type, visiting);
        }
        if (const auto* b = dynamic_cast<const ir::Bits*>(m.get())) {
            for (const auto& f : b->fields) if (f && f->symbol == id) {
                std::unordered_set<core::SymbolId> visiting;
                return expressionTypeFromType(src, f->type, visiting);
            }
        }
        if (const auto* c = dynamic_cast<const ir::Conditional*>(m.get())) {
            auto result = expressionTypeFromMembers(src, c->thenMembers, id);
            if (result != core::ExprType::Invalid) return result;
            result = expressionTypeFromMembers(src, c->elseMembers, id);
            if (result != core::ExprType::Invalid) return result;
        }
        if (const auto* v = dynamic_cast<const ir::Variant*>(m.get())) {
            for (const auto& c : v->cases) {
                auto result = expressionTypeFromMembers(src, c.members, id);
                if (result != core::ExprType::Invalid) return result;
            }
            if (v->hasDefault) {
                auto result = expressionTypeFromMembers(src, v->defaultMembers, id);
                if (result != core::ExprType::Invalid) return result;
            }
        }
        if (const auto* b = dynamic_cast<const ir::Block*>(m.get())) {
            auto result = expressionTypeFromMembers(src, b->members, id);
            if (result != core::ExprType::Invalid) return result;
        }
        if (const auto* a = dynamic_cast<const ir::At*>(m.get())) {
            auto result = expressionTypeFromMembers(src, a->members, id);
            if (result != core::ExprType::Invalid) return result;
        }
    }
    return core::ExprType::Invalid;
}

core::ExprType expressionTypeFromSymbol(const ir::Module& src, core::SymbolId id,
                                        const runtime::SymbolEnvironment& env) {
    const auto* sym = src.symbolTable.find(id);
    if (!sym) return core::ExprType::Invalid;
    if (sym->kind == core::SymbolKind::Constant) {
        const auto it = env.find(id);
        if (it == env.end()) return core::ExprType::Unknown;
        if (std::holds_alternative<std::int64_t>(it->second)) return core::ExprType::IntegerSigned;
        if (std::holds_alternative<std::uint64_t>(it->second)) return core::ExprType::IntegerUnsigned;
        if (std::holds_alternative<double>(it->second)) return core::ExprType::Floating;
        if (std::holds_alternative<bool>(it->second)) return core::ExprType::Boolean;
        return core::ExprType::Invalid;
    }
    if (sym->kind == core::SymbolKind::Parameter) {
        for (const auto& p : src.parameters) if (p.symbol == id) {
            std::unordered_set<core::SymbolId> visiting;
            return expressionTypeFromType(src, p.type, visiting);
        }
        return core::ExprType::Invalid;
    }
    if (sym->kind == core::SymbolKind::Field) {
        for (const auto& st : src.structs) {
            if (!st) continue;
            const auto result = expressionTypeFromMembers(src, st->members, id);
            if (result != core::ExprType::Invalid) return result;
        }
    }
    if (sym->kind == core::SymbolKind::AliasField) {
        for (const auto& st : src.structs) {
            if (!st) continue;
            std::function<const ir::FieldAlias*(const std::vector<std::unique_ptr<ir::Member>>&)> findAlias =
                [&](const auto& members) -> const ir::FieldAlias* {
                    for (const auto& m : members) {
                        if (!m) continue;
                        if (const auto* a = dynamic_cast<const ir::FieldAlias*>(m.get()); a && a->symbol == id) return a;
                        if (const auto* c = dynamic_cast<const ir::Conditional*>(m.get())) {
                            if (const auto* a = findAlias(c->thenMembers)) return a;
                            if (const auto* a = findAlias(c->elseMembers)) return a;
                        }
                        if (const auto* v = dynamic_cast<const ir::Variant*>(m.get())) {
                            for (const auto& x : v->cases) if (const auto* a = findAlias(x.members)) return a;
                            if (v->hasDefault) if (const auto* a = findAlias(v->defaultMembers)) return a;
                        }
                        if (const auto* b = dynamic_cast<const ir::Block*>(m.get())) if (const auto* a = findAlias(b->members)) return a;
                        if (const auto* a = dynamic_cast<const ir::At*>(m.get())) if (const auto* x = findAlias(a->members)) return x;
                    }
                    return nullptr;
                };
            if (const auto* a = findAlias(st->members)) {
                return expressionTypeFromSymbol(src, a->target, env);
            }
        }
    }
    if (sym->kind == core::SymbolKind::VirtualField) {
        // Virtual fields are expressions, not declarations with an independent
        // scalar type. Their type is resolved recursively by
        // resolvedExpressionType so identifier references inside the virtual
        // expression are resolved by SymbolId rather than by stale annotations.
        return core::ExprType::Unknown;
    }
    if (sym->kind == core::SymbolKind::EnumItem) {
        for (const auto& e : src.enums) for (const auto& item : e.items) if (item.symbol == id) {
            if (e.underlying.name.empty()) return core::ExprType::IntegerUnsigned;
            std::unordered_set<core::SymbolId> visiting;
            return expressionTypeFromType(src, e.underlying, visiting);
        }
    }
    return core::ExprType::Invalid;
}

core::ExprType resolvedExpressionType(const ir::Module& src, const core::Expr* e,
                                       const runtime::SymbolEnvironment& env,
                                       std::unordered_set<core::SymbolId>& visiting,
                                       std::string& err) {
    if (!e) { err = "missing expression"; return core::ExprType::Invalid; }
    if (e->kind == core::ExprKind::Identifier) {
        if (!e->reference.valid()) { err = "expression has invalid SymbolId: " + e->text; return core::ExprType::Invalid; }
        const auto id = e->reference.id;
        if (e->text == "$next") { if (id != core::BuiltinNextSymbolId) { err = "invalid builtin expression identity"; return core::ExprType::Invalid; } return core::ExprType::IntegerUnsigned; }
        if (e->text == "$size_in_bytes") { if (id != core::BuiltinSizeInBytesSymbolId) { err = "invalid builtin expression identity"; return core::ExprType::Invalid; } return core::ExprType::IntegerUnsigned; }
        if (e->text == "$min_size_in_bytes") { if (id != core::BuiltinMinSizeInBytesSymbolId) { err = "invalid builtin expression identity"; return core::ExprType::Invalid; } return core::ExprType::IntegerUnsigned; }
        if (e->text == "$max_size_in_bytes") { if (id != core::BuiltinMaxSizeInBytesSymbolId) { err = "invalid builtin expression identity"; return core::ExprType::Invalid; } return core::ExprType::IntegerUnsigned; }
        const auto* sym = src.symbolTable.find(id);
        if (!sym) { err = "expression references unknown SymbolId: " + e->text; return core::ExprType::Invalid; }
        if (sym->kind == core::SymbolKind::VirtualField) {
            if (!visiting.insert(id).second) { err = "cyclic virtual field expression: " + sym->name; return core::ExprType::Invalid; }
            for (const auto& st : src.structs) if (st) {
                std::function<const core::Expr*(const std::vector<std::unique_ptr<ir::Member>>&)> find =
                    [&](const auto& members) -> const core::Expr* {
                        for (const auto& m : members) if (m) {
                            if (const auto* v = dynamic_cast<const ir::Virtual*>(m.get()); v && v->symbol == id) return v->expression.get();
                            if (const auto* c = dynamic_cast<const ir::Conditional*>(m.get())) { if (auto* x=find(c->thenMembers)) return x; if (auto* x=find(c->elseMembers)) return x; }
                            if (const auto* v = dynamic_cast<const ir::Variant*>(m.get())) { for (const auto& c : v->cases) if (auto* x=find(c.members)) return x; if (v->hasDefault) if (auto* x=find(v->defaultMembers)) return x; }
                            if (const auto* b = dynamic_cast<const ir::Block*>(m.get())) if (auto* x=find(b->members)) return x;
                            if (const auto* a = dynamic_cast<const ir::At*>(m.get())) if (auto* x=find(a->members)) return x;
                        }
                        return nullptr;
                    };
                if (const auto* x = find(st->members)) { const auto result = resolvedExpressionType(src, x, env, visiting, err); visiting.erase(id); return result; }
            }
            visiting.erase(id);
            err = "virtual field SymbolId is absent from IR: " + sym->name;
            return core::ExprType::Invalid;
        }
        if (sym->kind == core::SymbolKind::AliasField) {
            for (const auto& st : src.structs) if (st) {
                std::function<const ir::FieldAlias*(const std::vector<std::unique_ptr<ir::Member>>&)> findAlias =
                    [&](const auto& members) -> const ir::FieldAlias* {
                        for (const auto& m : members) if (m) {
                            if (const auto* a = dynamic_cast<const ir::FieldAlias*>(m.get()); a && a->symbol == id) return a;
                            if (const auto* c = dynamic_cast<const ir::Conditional*>(m.get())) {
                                if (auto* x = findAlias(c->thenMembers)) return x;
                                if (auto* x = findAlias(c->elseMembers)) return x;
                            }
                            if (const auto* v = dynamic_cast<const ir::Variant*>(m.get())) {
                                for (const auto& c : v->cases) if (auto* x = findAlias(c.members)) return x;
                                if (v->hasDefault) if (auto* x = findAlias(v->defaultMembers)) return x;
                            }
                            if (const auto* b = dynamic_cast<const ir::Block*>(m.get())) if (auto* x = findAlias(b->members)) return x;
                            if (const auto* a = dynamic_cast<const ir::At*>(m.get())) if (auto* x = findAlias(a->members)) return x;
                        }
                        return nullptr;
                    };
                if (const auto* a = findAlias(st->members)) {
                    core::Expr target;
                    target.kind = core::ExprKind::Identifier;
                    target.text = a->targetName;
                    target.reference.id = a->target;
                    return resolvedExpressionType(src, &target, env, visiting, err);
                }
            }
            err = "alias field SymbolId is absent from IR: " + sym->name;
            return core::ExprType::Invalid;
        }
        const auto result = expressionTypeFromSymbol(src, id, env);
        if (result == core::ExprType::Unknown) { err = "expression type is unresolved: " + e->text; return core::ExprType::Invalid; }
        return result;
    }
    if (e->kind == core::ExprKind::Literal) return core::inferLiteralExprType(e->text, err);
    if (e->kind == core::ExprKind::Parenthesized) return resolvedExpressionType(src, e->left.get(), env, visiting, err);
    if (e->kind == core::ExprKind::Unary) {
        if (e->op != "-") { err = "unknown unary operator: " + e->op; return core::ExprType::Invalid; }
        const auto t = resolvedExpressionType(src, e->right.get(), env, visiting, err);
        if (t == core::ExprType::IntegerSigned || t == core::ExprType::IntegerUnsigned || t == core::ExprType::Floating)
            return t == core::ExprType::IntegerUnsigned ? core::ExprType::IntegerSigned : t;
        if (err.empty()) err = "unary '-' requires a numeric operand";
        return core::ExprType::Invalid;
    }
    if (e->kind == core::ExprKind::Binary) {
        const auto l = resolvedExpressionType(src, e->left.get(), env, visiting, err);
        err.clear();
        const auto r = resolvedExpressionType(src, e->right.get(), env, visiting, err);
        err.clear();
        const bool ln = l == core::ExprType::IntegerSigned || l == core::ExprType::IntegerUnsigned || l == core::ExprType::Floating;
        const bool rn = r == core::ExprType::IntegerSigned || r == core::ExprType::IntegerUnsigned || r == core::ExprType::Floating;
        if (e->op == "&&" || e->op == "||") {
            if (l != core::ExprType::Boolean || r != core::ExprType::Boolean) { err = "logical operator requires boolean operands"; return core::ExprType::Invalid; }
            return core::ExprType::Boolean;
        }
        if (e->op == "==" || e->op == "!=" || e->op == "<" || e->op == "<=" || e->op == ">" || e->op == ">=") {
            if (!ln || !rn) { err = "comparison operands must be numeric"; return core::ExprType::Invalid; }
            return core::ExprType::Boolean;
        }
        if (e->op != "+" && e->op != "-" && e->op != "*" && e->op != "/" && e->op != "%") { err = "unknown operator: " + e->op; return core::ExprType::Invalid; }
        if (!ln || !rn) { err = "arithmetic operator requires numeric operands"; return core::ExprType::Invalid; }
        if (e->op == "%" && (l == core::ExprType::Floating || r == core::ExprType::Floating)) { err = "% requires integer operands"; return core::ExprType::Invalid; }
        if (l == core::ExprType::Floating || r == core::ExprType::Floating) return core::ExprType::Floating;
        if (l == core::ExprType::IntegerUnsigned || r == core::ExprType::IntegerUnsigned) return core::ExprType::IntegerUnsigned;
        return core::ExprType::IntegerSigned;
    }
    err = "unknown expression kind";
    return core::ExprType::Invalid;
}

bool validateExpressionType(const ir::Module& src, core::Expr* e,
                            const runtime::SymbolEnvironment& env,
                            const std::string& owner, std::string& err,
                            std::optional<core::ExprType> required = std::nullopt) {
    std::unordered_set<core::SymbolId> visiting;
    const auto type = resolvedExpressionType(src, e, env, visiting, err);
    if (type == core::ExprType::Invalid) { err = owner + ": " + err; return false; }
    if (required && type != *required) {
        err = owner + ": expression has wrong type";
        return false;
    }
    e->type = type;
    return true;
}


bool materializeExecutionType(const ir::Type& input, const ir::Module& src,
                              const runtime::SymbolEnvironment& env, Type& out,
                              std::unordered_set<core::SymbolId>& visiting, std::string& err) {
    out = input;
    if (input.kind != core::TypeKind::Named || !input.reference.valid()) return true;

    const auto* sym = src.symbolTable.find(input.reference.id);
    if (!sym) { err = "type has unknown SymbolId: " + input.name; return false; }
    if (sym->kind != core::SymbolKind::TypeAlias) return true;

    const auto it = std::find_if(src.aliases.begin(), src.aliases.end(),
        [&](const ir::Alias& a) { return a.symbol == input.reference.id; });
    if (it == src.aliases.end()) { err = "type alias is absent from IR: " + input.name; return false; }
    if (!visiting.insert(input.reference.id).second) {
        err = "cyclic type alias during Plan materialization: " + input.name;
        return false;
    }

    Type target;
    if (!materializeExecutionType(it->target, src, env, target, visiting, err)) {
        visiting.erase(input.reference.id);
        return false;
    }
    // Alias dimensions are part of the aliased type; dimensions written at
    // the use site extend that shape.
    for (const auto& d : input.dimensions) target.dimensions.push_back(d);
    out = std::move(target);
    visiting.erase(input.reference.id);
    return true;
}

bool materializeExecutionType(const ir::Type& input, const ir::Module& src,
                              const runtime::SymbolEnvironment& env, Type& out, std::string& err) {
    std::unordered_set<core::SymbolId> visiting;
    if (!materializeExecutionType(input, src, env, out, visiting, err)) return false;
    // Canonicalize every statically evaluable fixed dimension to a literal.
    // Runtime expressions remain intact only when they genuinely depend on
    // runtime state (for example, a preceding field).
    for (auto& d : out.dimensions) {
        if (d.kind != core::Dimension::Kind::Fixed || !d.expression) continue;
        runtime::LayoutSize n = 0;
        std::string e;
        if (runtime::evaluateSize(d.expression.get(), env, n, e)) {
            auto lit = std::make_unique<core::Expr>();
            lit->kind = core::ExprKind::Literal;
            lit->text = std::to_string(n);
            d.expression = std::move(lit);
        } else if (!e.empty() && e.rfind("unknown identifier:", 0) != 0 && e.rfind("unknown symbol:", 0) != 0) {
            err = e;
            return false;
        }
    }
    return true;
}

Endian resolveEndian(core::Endian field, core::Endian outer, core::Endian module) {
    const auto cv=[](core::Endian e){return e==core::Endian::Big?Endian::Big:e==core::Endian::Native?Endian::Native:Endian::Little;};
    if(field!=core::Endian::Inherit) return cv(field);
    if(outer!=core::Endian::Inherit) return cv(outer);
    if(module!=core::Endian::Inherit) return cv(module);
    return Endian::Little;
}

std::optional<runtime::LayoutSize> primitiveSize(const std::string& n){
    if(n=="u8"||n=="i8") return 1;
    if(n=="u16"||n=="i16") return 2;
    if(n=="u32"||n=="i32"||n=="f32") return 4;
    if(n=="u64"||n=="i64"||n=="f64") return 8;
    return std::nullopt;
}

bool addChecked(runtime::LayoutSize a,runtime::LayoutSize b,runtime::LayoutSize& out){if(b>std::numeric_limits<runtime::LayoutSize>::max()-a)return false;out=a+b;return true;}

bool validateExprIdentity(const ir::Module& src, const core::Expr* e, const std::string& owner, std::string& err) {
    if (!e) return true;
    if (e->kind == core::ExprKind::Identifier) {
        if (e->text == "$next" || e->text == "$size_in_bytes" || e->text == "$min_size_in_bytes" || e->text == "$max_size_in_bytes") {
            if (e->reference.id != core::BuiltinNextSymbolId && e->reference.id != core::BuiltinSizeInBytesSymbolId && e->reference.id != core::BuiltinMinSizeInBytesSymbolId && e->reference.id != core::BuiltinMaxSizeInBytesSymbolId) { err = owner + ": invalid builtin expression identity"; return false; }
            return true;
        }
        if (!e->reference.valid()) { err = owner + ": unknown identifier: " + e->text; return false; }
        if (!src.symbolTable.find(e->reference.id)) { err = owner + ": identifier has unknown SymbolId: " + e->text; return false; }
    }
    return validateExprIdentity(src, e->left.get(), owner, err) &&
           validateExprIdentity(src, e->right.get(), owner, err);
}

bool validateTypeIdentity(const ir::Module& src, const ir::Type& t, const std::string& owner, std::string& err) {
    if (t.kind == core::TypeKind::Named) {
        if (!t.reference.valid()) { err = owner + ": named type has invalid SymbolId: " + t.name; return false; }
        const auto* sym = src.symbolTable.find(t.reference.id);
        if (!sym) { err = owner + ": named type has unknown SymbolId: " + t.name; return false; }
        if (sym->kind != core::SymbolKind::TypeAlias && sym->kind != core::SymbolKind::Struct && sym->kind != core::SymbolKind::Enum) {
            err = owner + ": SymbolId does not identify a type: " + t.name;
            return false;
        }
    }
    for (const auto& d : t.dimensions)
        if (!validateExprIdentity(src, d.expression.get(), owner, err)) return false;
    return true;
}

bool validateMemberIdentity(const ir::Module& src, const ir::Member* m, const std::string& owner, std::string& err) {
    if (!m) { err = owner + ": null IR member"; return false; }
    const auto validateSymbol = [&](core::SymbolId id, core::SymbolKind kind, const std::string& name, const std::string& what) {
        if (id == core::InvalidSymbolId) { err = owner + ": " + what + " has invalid SymbolId: " + name; return false; }
        const auto* sym = src.symbolTable.find(id);
        if (!sym || sym->kind != kind) { err = owner + ": " + what + " SymbolId is not executable: " + name; return false; }
        if (sym->name != name) { err = owner + ": " + what + " SymbolId/name mismatch: " + name; return false; }
        return true;
    };
    if (const auto* v = dynamic_cast<const ir::Virtual*>(m)) {
        if (!validateSymbol(v->symbol, core::SymbolKind::VirtualField, v->name, "virtual field")) return false;
        return validateExprIdentity(src, v->expression.get(), owner + "." + v->name, err);
    }
    if (const auto* a = dynamic_cast<const ir::FieldAlias*>(m)) {
        if (!validateSymbol(a->symbol, core::SymbolKind::AliasField, a->name, "alias field")) return false;
        if (a->target == core::InvalidSymbolId) { err = owner + "." + a->name + ": alias target has invalid SymbolId: " + a->targetName; return false; }
        const auto* target = src.symbolTable.find(a->target);
        if (!target || target->kind != core::SymbolKind::Field) { err = owner + "." + a->name + ": alias target SymbolId is not a field: " + a->targetName; return false; }
        if (target->name != a->targetName) { err = owner + "." + a->name + ": alias target SymbolId/name mismatch: " + a->targetName; return false; }
        return true;
    }
    if (const auto* f = dynamic_cast<const ir::Field*>(m)) {
        if (!validateSymbol(f->symbol, core::SymbolKind::Field, f->name, "field")) return false;
        return validateTypeIdentity(src, f->type, owner + "." + f->name, err);
    }
    if (const auto* b = dynamic_cast<const ir::Bits*>(m)) {
        for (const auto& f : b->fields) if (!validateMemberIdentity(src, f.get(), owner + ".<bits>", err)) return false;
        return true;
    }
    if (const auto* c = dynamic_cast<const ir::Conditional*>(m)) {
        if (!validateExprIdentity(src, c->condition.get(), owner + ".<if>", err)) return false;
        for (const auto& x : c->thenMembers)
            if (!validateMemberIdentity(src, x.get(), owner + ".<if>", err)) return false;
        for (const auto& x : c->elseMembers)
            if (!validateMemberIdentity(src, x.get(), owner + ".<else>", err)) return false;
        return true;
    }
    if (const auto* v = dynamic_cast<const ir::Variant*>(m)) {
        if (!validateExprIdentity(src, v->discriminator.get(), owner + "." + v->name, err)) return false;
        for (const auto& c : v->cases) {
            if (!validateExprIdentity(src, c.tag.get(), owner + "." + v->name, err)) return false;
            if (c.hasType) { if (!validateTypeIdentity(src, c.type, owner + "." + v->name, err)) return false; }
            else for (const auto& x : c.members) if (!validateMemberIdentity(src, x.get(), owner + "." + v->name, err)) return false;
        }
        if (v->hasDefault) {
            if (v->defaultType.name.empty()) { for (const auto& x : v->defaultMembers) if (!validateMemberIdentity(src, x.get(), owner + "." + v->name + ".default", err)) return false; }
            else if (!validateTypeIdentity(src, v->defaultType, owner + "." + v->name + ".default", err)) return false;
        }
        return true;
    }
    if (const auto* b = dynamic_cast<const ir::Block*>(m)) {
        if (!validateExprIdentity(src, b->size.get(), owner + ".<block>", err)) return false;
        for (const auto& x : b->members) if (!validateMemberIdentity(src, x.get(), owner + ".<block>", err)) return false;
        return true;
    }
    if (const auto* a = dynamic_cast<const ir::At*>(m)) {
        if (!validateExprIdentity(src, a->offset.get(), owner + ".<at>", err)) return false;
        for (const auto& x : a->members) if (!validateMemberIdentity(src, x.get(), owner + ".<at>", err)) return false;
        return true;
    }
    if (const auto* a = dynamic_cast<const ir::Align*>(m)) return validateExprIdentity(src, a->alignment.get(), owner + ".<align>", err);
    if (const auto* c = dynamic_cast<const ir::Callback*>(m)) {
        for (const auto& e : c->args) if (!validateExprIdentity(src, e.get(), owner + "." + c->name, err)) return false;
        return true;
    }
    err = owner + ": unknown IR member kind";
    return false;
}

bool validateIRIdentity(const ir::Module& src, std::string& err) {
    for (const auto& a : src.aliases) if (!validateTypeIdentity(src, a.target, "alias " + a.name, err)) return false;
    for (const auto& e : src.enums) {
        if (e.hasUnderlying && !validateTypeIdentity(src, e.underlying, "enum " + e.name, err)) return false;
        for (const auto& i : e.items) if (!validateExprIdentity(src, i.value.get(), "enum " + e.name + " item " + i.name, err)) return false;
    }
    for (const auto& c : src.constants) if (!validateExprIdentity(src, c.expr.get(), "constant " + c.name, err)) return false;
    for (const auto& s : src.structs) {
        if (!s) { err = "null IR struct"; return false; }
        for (const auto& m : s->members) if (!validateMemberIdentity(src, m.get(), "struct " + s->name, err)) return false;
    }
    return true;
}

bool lowerMembers(const std::vector<std::unique_ptr<ir::Member>>& members, const ir::Module& src, core::Endian outer, core::Endian module, const runtime::SymbolEnvironment& env, bool bounded, std::vector<std::unique_ptr<Op>>& dst, std::string& err);

bool dynamicStarType(const ir::Type& t, const ir::Module& src, std::unordered_set<core::SymbolId>& visiting) {
    if (core::hasRemaining(t)) return true;
    if (!t.reference.valid()) return false;
    const auto it = std::find_if(src.aliases.begin(), src.aliases.end(),
        [&](const ir::Alias& a) { return a.symbol == t.reference.id; });
    if (it == src.aliases.end()) return false;
    if (!visiting.insert(it->symbol).second) return false;
    const bool result = dynamicStarType(it->target, src, visiting);
    visiting.erase(it->symbol);
    return result;
}

bool lowerOne(const ir::Member* m, const ir::Module& src, core::Endian outer, core::Endian module, const runtime::SymbolEnvironment& env, bool bounded, std::unique_ptr<Op>& out, std::string& err) {
    if(!m){err="null IR member";return false;}
    if(auto* v=dynamic_cast<const ir::Virtual*>(m)){
        auto x=std::make_unique<Virtual>(); x->symbol=v->symbol; x->attributes=v->attributes; x->documentation=v->documentation; x->name=v->name; x->expression=core::cloneExpr(v->expression.get()); out=std::move(x); return true;
    }
    if(auto* a=dynamic_cast<const ir::FieldAlias*>(m)){
        auto x=std::make_unique<FieldAlias>(); x->symbol=a->symbol; x->attributes=a->attributes; x->documentation=a->documentation; x->name=a->name; x->target=a->target; x->targetName=a->targetName; out=std::move(x); return true;
    }
    if(auto* f=dynamic_cast<const ir::Field*>(m)){
        if (!f->type.terminator.empty()) {
            if (f->type.kind != core::TypeKind::Bytes) { err = "terminated sequence is only valid for bytes: " + f->name; return false; }
            if (!f->type.maxPayload.has_value()) { err = "terminated sequence requires a maximum payload length: " + f->name; return false; }
            if (!f->type.dimensions.empty()) { err = "terminated sequence cannot have an array suffix: " + f->name; return false; }
            if (*f->type.maxPayload > std::numeric_limits<runtime::LayoutSize>::max() - static_cast<runtime::LayoutSize>(f->type.terminator.size())) { err = "terminated sequence maximum length overflow: " + f->name; return false; }
        }
        if (f->symbol == core::InvalidSymbolId || !src.symbolTable.find(f->symbol) || src.symbolTable.find(f->symbol)->kind != core::SymbolKind::Field) { err="field has invalid SymbolId: "+f->name; return false; }
        auto x=std::make_unique<Field>(); x->symbol=f->symbol; x->attributes=f->attributes; x->documentation=f->documentation; x->name=f->name; if (!materializeExecutionType(f->type, src, env, x->type, err)) return false;
        if (!x->type.terminator.empty() && !x->type.dimensions.empty()) { err = "terminated sequence cannot have an array suffix: " + f->name; return false; }
        x->bits=f->bits; x->endian=resolveEndian(f->endian,outer,module); x->assertion=f->assertion;
        if (f->transform) {
            if (f->transform->name != "scale" || f->transform->arguments.size() != 1) { err = "invalid scale transform: " + f->name; return false; }
            runtime::Value factor;
            if (!evaluateExpr(f->transform->arguments[0].get(), env, factor, err)) { err = "scale factor is not a compile-time constant: " + f->name + ": " + err; return false; }
            double d = 0.0;
            if (auto p = std::get_if<double>(&factor)) d=*p; else if (auto p=std::get_if<int64_t>(&factor)) d=static_cast<double>(*p); else if (auto p=std::get_if<uint64_t>(&factor)) d=static_cast<double>(*p); else { err="scale factor must be numeric: "+f->name; return false; }
            if (!std::isfinite(d) || d == 0.0) { err="scale factor must be finite and non-zero: "+f->name; return false; }
            x->transform = Transform{"scale", d};
        }
        const bool hasTypeShape = !f->type.dimensions.empty();
        if (f->bits != 0) { err="bit width is only valid inside a bits block: "+f->name; return false; }
        if(!bounded){std::unordered_set<core::SymbolId> visiting;if(dynamicStarType(f->type,src,visiting)){err="bytes[*]/string[*] requires a bounded context: "+f->name;return false;}}
        if (hasTypeShape) {
            runtime::LayoutSize count = 1;
            bool fixed = true;
            for (const auto& suffix : f->type.dimensions) {
                if (suffix.kind != core::Dimension::Kind::Fixed) { fixed = false; break; }
                runtime::LayoutSize n = 0; std::string e;
                if (!suffix.expression || !runtime::evaluateSize(suffix.expression.get(), env, n, e)) {
                    if (!e.empty() && e.rfind("unknown identifier:", 0) != 0 && e.rfind("unknown symbol:", 0) != 0) { err = e; return false; }
                    fixed = false; break;
                }
                if (n != 0 && count > std::numeric_limits<runtime::LayoutSize>::max() / n) {
                    err = "array size overflow"; return false;
                }
                count *= n;
            }
            if (fixed) x->staticLength = count;
        }
        out=std::move(x);return true;
    }
    if(auto* b=dynamic_cast<const ir::Bits*>(m)){
        auto x=std::make_unique<Bits>(); x->attributes=b->attributes; x->documentation=b->documentation; int total=0;for(const auto& f:b->fields){if(!f){err="null IR bit field";return false;} if (f->symbol == core::InvalidSymbolId || !src.symbolTable.find(f->symbol) || src.symbolTable.find(f->symbol)->kind != core::SymbolKind::Field) { err="bit field has invalid SymbolId: "+f->name; return false; } BitsField q;q.symbol=f->symbol;q.attributes=f->attributes;q.name=f->name;q.documentation=f->documentation;if (!materializeExecutionType(f->type, src, env, q.type, err)) return false;q.endian=resolveEndian(f->endian,outer,module);q.bits=f->bits;q.assertion=f->assertion;total+=q.bits;x->fields.push_back(std::move(q));}if(total<1||total>64){err="bits block must contain 1..64 bits";return false;}x->totalBits=total;x->storageBytes=static_cast<runtime::LayoutSize>((total+7)/8);out=std::move(x);return true;
    }
    if(auto* c=dynamic_cast<const ir::Conditional*>(m)){
        auto x=std::make_unique<Conditional>(); x->attributes=c->attributes; x->documentation=c->documentation; x->condition=core::cloneExpr(c->condition.get());
        if(!lowerMembers(c->thenMembers,src,outer,module,env,bounded,x->thenMembers,err)) return false;
        if(!lowerMembers(c->elseMembers,src,outer,module,env,bounded,x->elseMembers,err)) return false;
        out=std::move(x); return true;
    }
    if(auto* v=dynamic_cast<const ir::Variant*>(m)){
        auto x=std::make_unique<Variant>();x->attributes=v->attributes;x->documentation=v->documentation;x->name=v->name;x->discriminator=core::cloneExpr(v->discriminator.get());
        if (!x->discriminator) { err="variant has no discriminator"; return false; }
        std::unordered_set<core::SymbolId> typeVisiting;
        std::string typeError;
        const auto discriminatorType = resolvedExpressionType(src, x->discriminator.get(), env, typeVisiting, typeError);
        if (discriminatorType == core::ExprType::Invalid) {
            err = typeError.empty() ? "variant discriminator must be an integer expression"
                                    : "variant discriminator: " + typeError;
            return false;
        }
        x->discriminator->type = discriminatorType;
        if (discriminatorType != core::ExprType::IntegerSigned && discriminatorType != core::ExprType::IntegerUnsigned) { err="variant discriminator must be an integer expression"; return false; }
        std::set<std::pair<int, std::uint64_t>> tags;
        for(const auto& c:v->cases){
            if(!c.tag){err="variant case missing tag";return false;}
            runtime::Value tag;
            std::string tagError;
            if(!evaluateExpr(c.tag.get(), env, tag, tagError)) {
                if(tagError.rfind("unknown identifier:",0)==0 || tagError.rfind("unknown symbol:",0)==0) err="variant case tag must be a compile-time constant";
                else err="variant case tag is not a valid compile-time integer: "+tagError;
                return false;
            }
            if(!std::holds_alternative<std::int64_t>(tag) && !std::holds_alternative<std::uint64_t>(tag)) { err="variant case tag must be an integer"; return false; }
            std::uint64_t key=0; int sign=0;
            if(auto sv=std::get_if<std::int64_t>(&tag)){
                if(x->discriminator->type==core::ExprType::IntegerUnsigned && *sv<0){err="variant case tag out of range";return false;}
                if(x->discriminator->type==core::ExprType::IntegerUnsigned){
                    key=static_cast<std::uint64_t>(*sv); sign=1; tag=key;
                } else {
                    key=static_cast<std::uint64_t>(*sv); sign=*sv<0?-1:1;
                }
            } else {
                const auto uv=std::get<std::uint64_t>(tag);
                if(x->discriminator->type==core::ExprType::IntegerSigned && uv>static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())){err="variant case tag out of range";return false;}
                if(x->discriminator->type==core::ExprType::IntegerSigned){
                    tag=static_cast<std::int64_t>(uv); key=uv; sign=1;
                } else {
                    key=uv; sign=1;
                }
            }
            if(!tags.insert({sign,key}).second){err="duplicate variant tag: "+(std::holds_alternative<std::int64_t>(tag)?std::to_string(std::get<std::int64_t>(tag)):std::to_string(std::get<std::uint64_t>(tag)));return false;}
            VariantCase q;q.tag=tag;
            if(c.hasType){if(!bounded){std::unordered_set<core::SymbolId> visiting;if(dynamicStarType(c.type,src,visiting)){err="bytes[*]/string[*] requires a bounded context in variant";return false;}}q.type.emplace(); if (!materializeExecutionType(c.type, src, env, *q.type, err)) return false;}else if(!lowerMembers(c.members,src,outer,module,env,bounded,q.members,err))return false;x->cases.push_back(std::move(q));}
        x->hasDefault=v->hasDefault;if(v->hasDefault){if(v->defaultType.name.empty()){if(!lowerMembers(v->defaultMembers,src,outer,module,env,bounded,x->defaultMembers,err))return false;}else {if(!bounded){std::unordered_set<core::SymbolId> visiting;if(dynamicStarType(v->defaultType,src,visiting)){err="bytes[*]/string[*] requires a bounded context in variant default";return false;}}x->defaultType.emplace(); if (!materializeExecutionType(v->defaultType, src, env, *x->defaultType, err)) return false;}}out=std::move(x);return true;
    }
    if(auto* b=dynamic_cast<const ir::Block*>(m)){
        auto x=std::make_unique<Block>();x->attributes=b->attributes;x->documentation=b->documentation;x->name=b->name;x->size=core::cloneExpr(b->size.get());std::string e;runtime::LayoutSize v=0;if(evaluateExprSize(b->size.get(),env,v,e))x->staticSize=v;else if(e.rfind("unknown identifier:",0)!=0 && e.rfind("unknown symbol:",0)!=0){err=e;return false;}if(!lowerMembers(b->members,src,outer,module,env,true,x->members,err))return false;out=std::move(x);return true;
    }
    if(auto* a=dynamic_cast<const ir::At*>(m)){
        auto x=std::make_unique<At>();x->attributes=a->attributes;x->documentation=a->documentation;x->offset=core::cloneExpr(a->offset.get());std::string e;runtime::LayoutSize v=0;if(evaluateExprSize(a->offset.get(),env,v,e))x->staticOffset=v;else if(e.rfind("unknown identifier:",0)!=0 && e.rfind("unknown symbol:",0)!=0){err=e;return false;}if(!lowerMembers(a->members,src,outer,module,env,bounded,x->members,err))return false;out=std::move(x);return true;
    }
    if(auto* a=dynamic_cast<const ir::Align*>(m)){
        auto x=std::make_unique<Align>();x->attributes=a->attributes;x->documentation=a->documentation;x->alignment=core::cloneExpr(a->alignment.get());runtime::LayoutSize v=0;if(!evaluateExprSize(a->alignment.get(),env,v,err))return false;if(v==0){err="alignment cannot be zero";return false;}x->staticAlignment=v;out=std::move(x);return true;
    }
    if(auto* c=dynamic_cast<const ir::Callback*>(m)){
        auto x=std::make_unique<Callback>();x->attributes=c->attributes;x->documentation=c->documentation;x->name=c->name;x->direction=c->direction;for(const auto& e:c->args)x->args.push_back(core::cloneExpr(e.get()));out=std::move(x);return true;
    }
    err="unknown IR member kind";return false;
}

bool lowerMembers(const std::vector<std::unique_ptr<ir::Member>>& members, const ir::Module& src, core::Endian outer, core::Endian module, const runtime::SymbolEnvironment& env, bool bounded, std::vector<std::unique_ptr<Op>>& dst, std::string& err){
    for(const auto& m:members){std::unique_ptr<Op> x;if(!lowerOne(m.get(),src,outer,module,env,bounded,x,err))return false;dst.push_back(std::move(x));}return true;
}

const Struct* findStructByType(const Module& p, const Type& t) {
    if (!t.reference.valid()) return nullptr;
    auto it = p.structIndexBySymbol.find(t.reference.id);
    return it == p.structIndexBySymbol.end() ? nullptr : &p.structs[it->second];
}

const Alias* findAliasByType(const Module& p, const Type& t) {
    if (!t.reference.valid()) return nullptr;
    auto it = p.aliasIndexBySymbol.find(t.reference.id);
    return it == p.aliasIndexBySymbol.end() ? nullptr : &p.aliases[it->second];
}

bool resolveIntegerPrimitive(const Module& p, const Type& t, std::string& base,
                             std::unordered_set<core::SymbolId>& visiting, std::string& err);

bool fixedTypeSizeImpl(const Module& p,const Type& t,runtime::LayoutSize& out,std::unordered_set<core::SymbolId>& visiting,std::string& err){
    runtime::LayoutSize base=0;
    if (!t.terminator.empty()) return false;
    if(auto s=primitiveSize(t.name))base=*s;
    else if(t.name=="bytes"||t.name=="string")base=1;
    else {
        const auto* sp = findStructByType(p, t);
        if(sp){
            if (!t.reference.valid()) { err="named type has no SymbolId: "+t.name; return false; }
            const core::SymbolId visitKey = t.reference.id;
            if(!visiting.insert(visitKey).second){err="cyclic struct reference in plan: "+t.name;return false;}
            runtime::LayoutSize total=0;
            for(const auto& op:sp->members){
                if(op->kind==OpKind::Alias){}
                else if(op->kind==OpKind::Field){auto* f=static_cast<const Field*>(op.get());runtime::LayoutSize fs=0;if(!fixedTypeSizeImpl(p,f->type,fs,visiting,err)){visiting.erase(visitKey);return false;}// bytes/string dimensions are byte/string lengths already included by fixedTypeSize;
                // do not multiply by staticLength a second time.
                if(!addChecked(total,fs,total)){err="fixed layout overflow";visiting.erase(visitKey);return false;}}
                else if(op->kind==OpKind::Bits){if(!addChecked(total,static_cast<const Bits*>(op.get())->storageBytes,total)){err="fixed layout overflow";visiting.erase(visitKey);return false;}}
                else if(op->kind==OpKind::Align){auto* a=static_cast<const Align*>(op.get());if(!a->staticAlignment){visiting.erase(visitKey);return false;}runtime::LayoutSize rem=total%*a->staticAlignment;if(rem&&!addChecked(total,*a->staticAlignment-rem,total)){err="fixed layout overflow";visiting.erase(visitKey);return false;}}
                else {visiting.erase(visitKey);return false;}
            }
            visiting.erase(visitKey);base=total;
        } else {
            // Enums are scalar wire values. Their layout size is the size of
            // the resolved integer underlying type, not a struct/alias size.
            const plan::Enum* ep = nullptr;
            if (t.reference.valid()) {
                auto ei = p.enumIndexBySymbol.find(t.reference.id);
                if (ei != p.enumIndexBySymbol.end()) ep = &p.enums[ei->second];
            }
            if (ep) {
                std::string underlying;
                std::unordered_set<core::SymbolId> enumVisiting;
                if (!resolveIntegerPrimitive(p, ep->underlying, underlying, enumVisiting, err)) return false;
                const auto sz = primitiveSize(underlying);
                if (!sz) { err = "invalid enum underlying type in plan: " + ep->name; return false; }
                base = *sz;
            } else {
            const auto* ap = findAliasByType(p, t);
            if(!ap){err="unknown named type in plan: "+t.name;return false;}
            if (!t.reference.valid()) { err="named type has no SymbolId: "+t.name; return false; }
            const core::SymbolId visitKey = t.reference.id;
            if(!visiting.insert(visitKey).second){err="cyclic alias in plan: "+t.name;return false;}
            bool ok=fixedTypeSizeImpl(p,ap->target,base,visiting,err);
            visiting.erase(visitKey);
            if(!ok)return false;
            }
        }
    }
    for(const auto& d:t.dimensions){if(d.kind!=core::Dimension::Kind::Fixed)return false;if(!d.expression)return false;runtime::LayoutSize n=0;std::string e2;if(!runtime::evaluateSize(d.expression.get(),runtime::SymbolEnvironment{},n,e2))return false;if(base&&n>std::numeric_limits<runtime::LayoutSize>::max()/base){err="fixed type size overflow";return false;}base*=n;}
    out=base;return true;
}



bool resolveIntegerPrimitive(const Module& p, const Type& t, std::string& base,
                             std::unordered_set<core::SymbolId>& visiting, std::string& err) {
    if (!t.dimensions.empty()) {
        err = "enum underlying type must be scalar integer";
        return false;
    }
    const auto isInt = [](const std::string& n) {
        return n=="u8"||n=="i8"||n=="u16"||n=="i16"||n=="u32"||n=="i32"||n=="u64"||n=="i64";
    };
    if (isInt(t.name)) { base=t.name; return true; }
    if (!t.reference.valid()) { err="enum underlying type is unresolved"; return false; }
    const auto it=p.aliasIndexBySymbol.find(t.reference.id);
    if (it==p.aliasIndexBySymbol.end()) { err="enum underlying type must resolve to a scalar integer"; return false; }
    const core::SymbolId key=t.reference.id;
    if (!visiting.insert(key).second) { err="cyclic alias in enum underlying type: "+t.name; return false; }
    const bool ok=resolveIntegerPrimitive(p,p.aliases[it->second].target,base,visiting,err);
    visiting.erase(key);
    return ok;
}

bool enumValueFits(const runtime::Value& v, const std::string& base, std::string& err) {
    const bool isUnsigned = !base.empty() && base[0]=='u';
    int bits = 0;
    if (base.size()>1) bits=std::stoi(base.substr(1));
    if (bits==0) { err="invalid enum underlying integer type: "+base; return false; }
    if (auto p=std::get_if<double>(&v)) { (void)p; err="enum value must be an integer"; return false; }
    if (auto p=std::get_if<bool>(&v)) { (void)p; err="enum value must be an integer"; return false; }
    if (isUnsigned) {
        if (auto p=std::get_if<int64_t>(&v)) {
            if (*p<0) { err="enum value is negative for unsigned underlying type"; return false; }
            const uint64_t max = bits==64 ? std::numeric_limits<uint64_t>::max() : ((uint64_t(1)<<bits)-1);
            if (uint64_t(*p)>max) { err="enum value exceeds underlying type range"; return false; }
        } else {
            const uint64_t x=std::get<uint64_t>(v);
            const uint64_t max = bits==64 ? std::numeric_limits<uint64_t>::max() : ((uint64_t(1)<<bits)-1);
            if (x>max) { err="enum value exceeds underlying type range"; return false; }
        }
    } else {
        const int64_t min = bits==64 ? std::numeric_limits<int64_t>::min() : -(int64_t(1)<<(bits-1));
        const int64_t max = bits==64 ? std::numeric_limits<int64_t>::max() : (int64_t(1)<<(bits-1))-1;
        if (auto p=std::get_if<int64_t>(&v)) {
            if (*p<min || *p>max) { err="enum value exceeds underlying type range"; return false; }
        } else {
            const uint64_t x=std::get<uint64_t>(v);
            if (x>uint64_t(max)) { err="enum value exceeds underlying type range"; return false; }
        }
    }
    return true;
}

void collectExprSymbols(const Expr* e, std::unordered_set<core::SymbolId>& ids, std::string& error) {
    if (!e) return;
    if (e->kind == core::ExprKind::Identifier) {
        if (e->text == "$next" || e->text == "$size_in_bytes" || e->text == "$min_size_in_bytes" || e->text == "$max_size_in_bytes") {
            collectExprSymbols(e->left.get(), ids, error);
            collectExprSymbols(e->right.get(), ids, error);
            return;
        }
        if (!e->reference.valid()) {
            error = "expression contains unresolved identifier: " + e->text;
            return;
        }
        ids.insert(e->reference.id);
    }
    collectExprSymbols(e->left.get(), ids, error);
    collectExprSymbols(e->right.get(), ids, error);
}

bool validateExprDependencies(const Module& p, const Expr* e,
                              const std::unordered_set<core::SymbolId>& available,
                              const std::string& owner, std::string& err) {
    std::unordered_set<core::SymbolId> ids;
    collectExprSymbols(e, ids, err);
    if (!err.empty()) return false;
    for (const auto id : ids) {
        if (available.count(id)) continue;
        const auto* symbol = p.symbolTable.find(id);
        if (!symbol) {
            err = owner + ": expression references unknown SymbolId";
            return false;
        }
        if (symbol->kind == core::SymbolKind::Field || symbol->kind == core::SymbolKind::VirtualField) {
            err = owner + ": value dependency declared later: " + symbol->name;
            return false;
        }
        if (symbol->kind == core::SymbolKind::Parameter) continue;
        if (symbol->kind != core::SymbolKind::Constant && symbol->kind != core::SymbolKind::EnumItem) {
            err = owner + ": expression references a non-value symbol: " + symbol->name;
            return false;
        }
        // Module constants and enum items are only valid once their values have
        // been materialized into the plan environment. The frontend guarantees
        // the reference identity; this phase enforces execution order.
        if (symbol->kind == core::SymbolKind::Constant) {
            bool present = false;
            for (const auto& c : p.constants) if (c.symbol == id) { present = true; break; }
            if (!present) { err = owner + ": expression references unavailable constant: " + symbol->name; return false; }
        }
    }
    return true;
}

bool validateTypeDependencies(const Module& p, const Type& t,
                              const std::unordered_set<core::SymbolId>& available,
                              const std::string& owner, std::string& err) {
    for (const auto& d : t.dimensions)
        if (!validateExprDependencies(p, d.expression.get(), available, owner, err)) return false;
    return true;
}

bool validateMembersDependencies(const Module& p,
                                 const std::vector<std::unique_ptr<Op>>& members,
                                 std::unordered_set<core::SymbolId>& available,
                                 const std::string& owner,
                                 std::string& err) {
    for (const auto& op : members) {
        if (!op) { err = owner + ": null plan operation"; return false; }
        switch (op->kind) {
        case OpKind::Virtual: {
            const auto& v = static_cast<const Virtual&>(*op);
            if (!validateExprDependencies(p, v.expression.get(), available, owner + "." + v.name, err)) return false;
            if (v.symbol == core::InvalidSymbolId) { err = owner + ": virtual field has invalid SymbolId: " + v.name; return false; }
            available.insert(v.symbol);
            break;
        }
        case OpKind::Alias: {
            const auto& a = static_cast<const FieldAlias&>(*op);
            if (a.symbol == core::InvalidSymbolId || a.target == core::InvalidSymbolId) { err = owner + ": alias field has invalid SymbolId: " + a.name; return false; }
            const auto* target = p.symbolTable.find(a.target);
            if (!target || (target->kind != core::SymbolKind::Field)) { err = owner + "." + a.name + ": alias target is not a field: " + a.targetName; return false; }
            if (!available.count(a.target)) { err = owner + "." + a.name + ": alias target declared later: " + a.targetName; return false; }
            available.insert(a.symbol);
            break;
        }
        case OpKind::Field: {
            const auto& f = static_cast<const Field&>(*op);
            if (!validateTypeDependencies(p, f.type, available, owner + "." + f.name, err)) return false;
            if (f.symbol == core::InvalidSymbolId) { err = owner + ": field has invalid SymbolId: " + f.name; return false; }
            available.insert(f.symbol);
            break;
        }
        case OpKind::Bits: {
            const auto& b = static_cast<const Bits&>(*op);
            for (const auto& bf : b.fields) {
                if (!validateTypeDependencies(p, bf.type, available, owner + "." + bf.name, err)) return false;
                if (bf.symbol == core::InvalidSymbolId) { err = owner + ": bits field has invalid SymbolId: " + bf.name; return false; }
                available.insert(bf.symbol);
            }
            break;
        }
        case OpKind::Align:
            if (!validateExprDependencies(p, static_cast<const Align&>(*op).alignment.get(), available, owner + ".<align>", err)) return false;
            break;
        case OpKind::At: {
            const auto& a = static_cast<const At&>(*op);
            if (!validateExprDependencies(p, a.offset.get(), available, owner + ".<at>", err)) return false;
            if (!validateMembersDependencies(p, a.members, available, owner + ".<at>", err)) return false;
            break;
        }
        case OpKind::Block: {
            const auto& b = static_cast<const Block&>(*op);
            if (!validateExprDependencies(p, b.size.get(), available, owner + ".<block>", err)) return false;
            if (!validateMembersDependencies(p, b.members, available, owner + ".<block>", err)) return false;
            break;
        }
        case OpKind::Conditional: {
            const auto& c = static_cast<const Conditional&>(*op);
            if (!validateExprDependencies(p, c.condition.get(), available, owner + ".<if>", err)) return false;
            auto thenAvailable = available;
            if (!validateMembersDependencies(p, c.thenMembers, thenAvailable, owner + ".<if>", err)) return false;
            auto elseAvailable = available;
            if (!validateMembersDependencies(p, c.elseMembers, elseAvailable, owner + ".<else>", err)) return false;
            break;
        }
        case OpKind::Variant: {
            const auto& v = static_cast<const Variant&>(*op);
            if (!validateExprDependencies(p, v.discriminator.get(), available, owner + "." + v.name, err)) return false;
            for (const auto& c : v.cases) {
                auto branch = available;
                if (c.type) {
                    if (!validateTypeDependencies(p, *c.type, branch, owner + "." + v.name, err)) return false;
                } else if (!validateMembersDependencies(p, c.members, branch, owner + "." + v.name, err)) return false;
            }
            if (v.defaultType && !validateTypeDependencies(p, *v.defaultType, available, owner + "." + v.name, err)) return false;
            if (!v.defaultType && v.hasDefault) {
                auto branch = available;
                if (!validateMembersDependencies(p, v.defaultMembers, branch, owner + "." + v.name, err)) return false;
            }
            break;
        }
        case OpKind::Callback: {
            const auto& c = static_cast<const Callback&>(*op);
            for (const auto& e : c.args)
                if (!validateExprDependencies(p, e.get(), available, owner + "." + c.name, err)) return false;
            break;
        }
        }
    }
    return true;
}



bool validatePlanExprIdentity(const Module& p, const core::Expr* e, const std::string& owner, std::string& err) {
    if (!e) return true;
    if (e->kind == core::ExprKind::Identifier) {
        if (!e->reference.valid()) { err = owner + ": expression has invalid SymbolId: " + e->text; return false; }
        if (e->text == "$next") {
            if (e->reference.id != core::BuiltinNextSymbolId) { err = owner + ": invalid builtin expression identity"; return false; }
        } else if (e->text == "$size_in_bytes") {
            if (e->reference.id != core::BuiltinSizeInBytesSymbolId) { err = owner + ": invalid builtin expression identity"; return false; }
        } else if (e->text == "$min_size_in_bytes") {
            if (e->reference.id != core::BuiltinMinSizeInBytesSymbolId) { err = owner + ": invalid builtin expression identity"; return false; }
        } else if (e->text == "$max_size_in_bytes") {
            if (e->reference.id != core::BuiltinMaxSizeInBytesSymbolId) { err = owner + ": invalid builtin expression identity"; return false; }
        } else if (!p.symbolTable.find(e->reference.id)) { err = owner + ": expression references unknown SymbolId: " + e->text; return false; }
    }
    return validatePlanExprIdentity(p, e->left.get(), owner, err) &&
           validatePlanExprIdentity(p, e->right.get(), owner, err);
}

bool validateExecutableType(const Module& p, const Type& t, const std::string& owner, std::string& err) {
    // bytes/string are built-in terminal types. They intentionally have no
    // SymbolId; only user-defined named types participate in identity checks.
    if (t.kind == core::TypeKind::Bytes || t.kind == core::TypeKind::String) {
        for (const auto& d : t.dimensions) {
            // [*] is a terminal runtime extent and intentionally has no expression.
            if (d.kind == core::Dimension::Kind::Remaining) continue;
            if (!d.expression) { err = owner + ": dimension has no expression"; return false; }
            if (!validatePlanExprIdentity(p, d.expression.get(), owner, err)) return false;
        }
        return true;
    }
    if (t.kind == core::TypeKind::Named) {
        if (!t.reference.valid()) { err = owner + ": named type has no SymbolId"; return false; }
        const auto* sym = p.symbolTable.find(t.reference.id);
        if (!sym) { err = owner + ": named type has unknown SymbolId"; return false; }
        if (sym->kind != core::SymbolKind::Struct &&
            sym->kind != core::SymbolKind::TypeAlias &&
            sym->kind != core::SymbolKind::Enum) {
            err = owner + ": named type SymbolId has non-type kind"; return false;
        }
        if (sym->kind == core::SymbolKind::Struct &&
            p.structIndexBySymbol.find(t.reference.id) == p.structIndexBySymbol.end()) {
            err = owner + ": struct type is absent from executable plan"; return false;
        }
        if (sym->kind == core::SymbolKind::TypeAlias &&
            p.aliasIndexBySymbol.find(t.reference.id) == p.aliasIndexBySymbol.end()) {
            err = owner + ": alias type is absent from executable plan"; return false;
        }
        if (sym->kind == core::SymbolKind::Enum &&
            p.enumIndexBySymbol.find(t.reference.id) == p.enumIndexBySymbol.end()) {
            err = owner + ": enum type is absent from executable plan"; return false;
        }
    }
    for (const auto& d : t.dimensions) {
        if (!d.expression) { err = owner + ": dimension has no expression"; return false; }
        std::string e;
        if (!validatePlanExprIdentity(p, d.expression.get(), owner, e)) {
            err = e;
            return false;
        }
    }
    return true;
}

bool validateExecutableMembers(const Module& p, const std::vector<std::unique_ptr<Op>>& members,
                               const std::string& owner, std::string& err) {
    for (const auto& op : members) {
        if (!op) { err = owner + ": null executable operation"; return false; }
        switch (op->kind) {
        case OpKind::Virtual: {
            const auto& v = static_cast<const Virtual&>(*op);
            if (v.symbol == core::InvalidSymbolId) { err = owner + ": virtual field has invalid SymbolId: " + v.name; return false; }
            const auto* sym = p.symbolTable.find(v.symbol);
            if (!sym || sym->kind != core::SymbolKind::VirtualField) { err = owner + ": virtual field SymbolId is not executable: " + v.name; return false; }
            if (!v.expression) { err = owner + ": virtual field has no expression: " + v.name; return false; }
            if (!validatePlanExprIdentity(p, v.expression.get(), owner + "." + v.name, err)) return false;
            break;
        }
        case OpKind::Alias: {
            const auto& a = static_cast<const FieldAlias&>(*op);
            if (a.symbol == core::InvalidSymbolId || a.target == core::InvalidSymbolId) { err = owner + ": alias field has invalid SymbolId: " + a.name; return false; }
            const auto* sym = p.symbolTable.find(a.symbol);
            const auto* target = p.symbolTable.find(a.target);
            if (!sym || sym->kind != core::SymbolKind::AliasField) { err = owner + ": alias field SymbolId is not executable: " + a.name; return false; }
            if (!target || (target->kind != core::SymbolKind::Field)) { err = owner + ": alias target is not a field: " + a.targetName; return false; }
            break;
        }
        case OpKind::Field: {
            const auto& f = static_cast<const Field&>(*op);
            if (f.symbol == core::InvalidSymbolId) { err = owner + ": field has invalid SymbolId: " + f.name; return false; }
            const auto* sym = p.symbolTable.find(f.symbol);
            if (!sym || sym->kind != core::SymbolKind::Field) { err = owner + ": field SymbolId is not executable: " + f.name; return false; }
            if (!validateExecutableType(p, f.type, owner + "." + f.name, err)) return false;
            if (f.staticLength && f.staticLength.value() == 0 && f.type.name != "bytes" && f.type.name != "string") {
                // Zero-sized fixed arrays are valid; keep the explicit value. No runtime work is needed.
            }
            break;
        }
        case OpKind::Bits: {
            const auto& b = static_cast<const Bits&>(*op);
            if (b.totalBits < 1 || b.totalBits > 64 || b.storageBytes != static_cast<runtime::LayoutSize>((b.totalBits + 7) / 8)) {
                err = owner + ": bits operation has inconsistent width metadata"; return false;
            }
            int total = 0;
            for (const auto& f : b.fields) {
                if (f.symbol == core::InvalidSymbolId) { err = owner + ": bits field has invalid SymbolId: " + f.name; return false; }
                const auto* sym = p.symbolTable.find(f.symbol);
                if (!sym || sym->kind != core::SymbolKind::Field) { err = owner + ": bits field SymbolId is not executable: " + f.name; return false; }
                if (f.bits <= 0 || f.bits > 64 || total > 64 - f.bits) { err = owner + ": bits field width is invalid: " + f.name; return false; }
                total += f.bits;
                if (!validateExecutableType(p, f.type, owner + "." + f.name, err)) return false;
            }
            if (total != b.totalBits) { err = owner + ": bits width does not match field widths"; return false; }
            break;
        }
        case OpKind::Conditional: {
            const auto& c = static_cast<const Conditional&>(*op);
            if (!c.condition) { err = owner + ": conditional has no condition"; return false; }
            if (c.condition->type != core::ExprType::Boolean) { err = owner + ": conditional condition is not boolean"; return false; }
            if (!validatePlanExprIdentity(p, c.condition.get(), owner + ".<if>", err)) return false;
            if (!validateExecutableMembers(p, c.thenMembers, owner + ".<if>", err)) return false;
            if (!validateExecutableMembers(p, c.elseMembers, owner + ".<else>", err)) return false;
            break;
        }
        case OpKind::Variant: {
            const auto& v = static_cast<const Variant&>(*op);
            if (!v.discriminator) { err = owner + ": variant has no discriminator"; return false; }
            if (!validatePlanExprIdentity(p, v.discriminator.get(), owner + "." + v.name, err)) return false;
            if (v.discriminator->type != core::ExprType::IntegerSigned && v.discriminator->type != core::ExprType::IntegerUnsigned) { err = owner + "." + v.name + ": discriminator is not integer"; return false; }
            std::set<std::pair<int, std::uint64_t>> seenTags;
            for (const auto& c : v.cases) {
                int sign=0; std::uint64_t key=0;
                if (const auto* sv = std::get_if<std::int64_t>(&c.tag)) { sign=*sv<0?-1:1; key=static_cast<std::uint64_t>(*sv); if(v.discriminator->type==core::ExprType::IntegerUnsigned && *sv<0){err=owner+"."+v.name+": case tag out of range";return false;} }
                else if (const auto* uv = std::get_if<std::uint64_t>(&c.tag)) { sign=1; key=*uv; if(v.discriminator->type==core::ExprType::IntegerSigned && *uv>static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())){err=owner+"."+v.name+": case tag out of range";return false;} }
                else { err = owner + "." + v.name + ": case tag is not integer"; return false; }
                if(!seenTags.insert({sign,key}).second){err=owner+"."+v.name+": duplicate variant tag";return false;}
                if (c.type) {
                    if (!validateExecutableType(p, *c.type, owner + "." + v.name, err)) return false;
                } else if (!validateExecutableMembers(p, c.members, owner + "." + v.name, err)) return false;
            }
            if (v.hasDefault) {
                if (v.defaultType) {
                    if (!validateExecutableType(p, *v.defaultType, owner + "." + v.name + ".default", err)) return false;
                } else if (!validateExecutableMembers(p, v.defaultMembers, owner + "." + v.name + ".default", err)) return false;
            }
            break;
        }
        case OpKind::Block: {
            const auto& b = static_cast<const Block&>(*op);
            if (!b.size) { err = owner + ": block has no size expression"; return false; }
            if (!validatePlanExprIdentity(p, b.size.get(), owner + ".<block>", err)) return false;
            if (b.staticSize && *b.staticSize > std::numeric_limits<runtime::LayoutSize>::max()) { err = owner + ": invalid block size"; return false; }
            if (!validateExecutableMembers(p, b.members, owner + ".<block>", err)) return false;
            break;
        }
        case OpKind::At: {
            const auto& a = static_cast<const At&>(*op);
            if (!a.offset) { err = owner + ": at has no offset expression"; return false; }
            if (!validatePlanExprIdentity(p, a.offset.get(), owner + ".<at>", err)) return false;
            if (!validateExecutableMembers(p, a.members, owner + ".<at>", err)) return false;
            break;
        }
        case OpKind::Align: {
            const auto& a = static_cast<const Align&>(*op);
            if (!a.alignment || !a.staticAlignment || *a.staticAlignment == 0) { err = owner + ": align is not finalized"; return false; }
            if (!validatePlanExprIdentity(p, a.alignment.get(), owner + ".<align>", err)) return false;
            break;
        }
        case OpKind::Callback: {
            const auto& c = static_cast<const Callback&>(*op);
            if (c.name.empty()) { err = owner + ": callback has no name"; return false; }
            for (const auto& e : c.args) {
                if (!e) { err = owner + "." + c.name + ": null callback argument"; return false; }
                if (!validatePlanExprIdentity(p, e.get(), owner + "." + c.name, err)) return false;
            }
            break;
        }
        }
    }
    return true;
}

bool validatePlanTypeExpressions(const ir::Module& src, Type& t,
                                  const runtime::SymbolEnvironment& env,
                                  const std::string& owner, std::string& err) {
    for (auto& d : t.dimensions) {
        if (d.kind == core::Dimension::Kind::Remaining) continue;
        if (!d.expression) { err = owner + ".dimension: missing expression"; return false; }
        if (!validateExpressionType(src, d.expression.get(), env, owner + ".dimension", err)) return false;
        if (d.expression->type != core::ExprType::IntegerSigned && d.expression->type != core::ExprType::IntegerUnsigned) { err = owner + ".dimension: expression has wrong type"; return false; }
    }
    return true;
}

bool validatePlanExpressionTypes(const ir::Module& src, Module& p,
                                 const runtime::SymbolEnvironment& env,
                                 std::string& err) {
    std::function<bool(std::vector<std::unique_ptr<Op>>&, const std::string&)> members =
        [&](auto& ops, const std::string& owner) -> bool {
            for (auto& op : ops) {
                if (!op) { err = owner + ": null plan operation"; return false; }
                switch (op->kind) {
                case OpKind::Virtual: {
                    auto& v = static_cast<Virtual&>(*op);
                    if (!validateExpressionType(src, v.expression.get(), env, owner + "." + v.name, err)) return false;
                    break;
                }
                case OpKind::Field: {
                    auto& f = static_cast<Field&>(*op);
                    if (!validatePlanTypeExpressions(src, f.type, env, owner + "." + f.name, err)) return false;
                    break;
                }
                case OpKind::Bits: {
                    auto& b = static_cast<Bits&>(*op);
                    for (auto& f : b.fields) if (!validatePlanTypeExpressions(src, f.type, env, owner + "." + f.name, err)) return false;
                    break;
                }
                case OpKind::Align: {
                    auto& a = static_cast<Align&>(*op);
                    if (!validateExpressionType(src, a.alignment.get(), env, owner + ".<align>", err)) return false;
                    if (a.alignment->type != core::ExprType::IntegerSigned && a.alignment->type != core::ExprType::IntegerUnsigned) { err = owner + ".<align>: expression has wrong type"; return false; }
                    break;
                }
                case OpKind::At: {
                    auto& a = static_cast<At&>(*op);
                    std::string local;
                    if (!validateExpressionType(src, a.offset.get(), env, owner + ".<at>", local)) { err = local; return false; }
                    const auto t = a.offset->type;
                    if (t != core::ExprType::IntegerSigned && t != core::ExprType::IntegerUnsigned) { err = owner + ".<at>: expression has wrong type"; return false; }
                    if (!members(a.members, owner + ".<at>")) return false;
                    break;
                }
                case OpKind::Block: {
                    auto& b = static_cast<Block&>(*op);
                    std::string local;
                    if (!validateExpressionType(src, b.size.get(), env, owner + ".<block>", local)) { err = local; return false; }
                    if (b.size->type != core::ExprType::IntegerSigned && b.size->type != core::ExprType::IntegerUnsigned) { err = owner + ".<block>: expression has wrong type"; return false; }
                    if (!members(b.members, owner + ".<block>")) return false;
                    break;
                }
                case OpKind::Conditional: {
                    auto& c = static_cast<Conditional&>(*op);
                    if (!validateExpressionType(src, c.condition.get(), env, owner + ".<if>", err, core::ExprType::Boolean)) return false;
                    if (!members(c.thenMembers, owner + ".<if>") || !members(c.elseMembers, owner + ".<else>")) return false;
                    break;
                }
                case OpKind::Variant: {
                    auto& v = static_cast<Variant&>(*op);
                    if (!validateExpressionType(src, v.discriminator.get(), env, owner + "." + v.name, err)) return false;
                    if (v.discriminator->type != core::ExprType::IntegerSigned && v.discriminator->type != core::ExprType::IntegerUnsigned) { err = owner + "." + v.name + ": discriminator is not integer"; return false; }
                    for (auto& c : v.cases) {
                        if (c.type && !validatePlanTypeExpressions(src, *c.type, env, owner + "." + v.name, err)) return false;
                        if (!c.type && !members(c.members, owner + "." + v.name)) return false;
                    }
                    if (v.defaultType && !validatePlanTypeExpressions(src, *v.defaultType, env, owner + "." + v.name + ".default", err)) return false;
                    if (!v.defaultType && v.hasDefault && !members(v.defaultMembers, owner + "." + v.name + ".default")) return false;
                    break;
                }
                case OpKind::Callback: {
                    auto& c = static_cast<Callback&>(*op);
                    for (auto& e : c.args) if (!validateExpressionType(src, e.get(), env, owner + "." + c.name, err)) return false;
                    break;
                }
                case OpKind::Alias:
                    break;
                }
            }
            return true;
        };
    for (auto& s : p.structs) {
        if (!members(s.members, s.name)) return false;
        for (auto& req : s.requirements) if (!validateExpressionType(src, req.get(), env, s.name + ".requires", err, core::ExprType::Boolean)) return false;
    }
    return true;
}

bool validateExecutablePlan(const Module& p, std::string& err) {
    err.clear();
    for (const auto& [sid, index] : p.structIndexBySymbol) {
        if (index >= p.structs.size() || p.structs[index].symbol != sid) { err = "struct index does not match SymbolId"; return false; }
    }
    for (const auto& [sid, index] : p.aliasIndexBySymbol) {
        if (index >= p.aliases.size() || p.aliases[index].symbol != sid) { err = "alias index does not match SymbolId"; return false; }
    }
    for (const auto& [sid, index] : p.enumIndexBySymbol) {
        if (index >= p.enums.size() || p.enums[index].symbol != sid) { err = "enum index does not match SymbolId"; return false; }
    }
    for (const auto& s : p.structs) {
        if (s.symbol == core::InvalidSymbolId) { err = "executable struct has invalid SymbolId: " + s.name; return false; }
        for (const auto& req : s.requirements) {
            if (!req) { err = "struct " + s.name + ": null requires expression"; return false; }
            if (req->type != core::ExprType::Boolean) { err = "struct " + s.name + ": requires expression must be boolean"; return false; }
            if (!validatePlanExprIdentity(p, req.get(), "struct " + s.name + ".requires", err)) return false;
        }
        if (!validateExecutableMembers(p, s.members, "struct " + s.name, err)) return false;
    }
    return true;
}

bool validateLayoutDependencies(const Module& p, std::string& err) {
    std::unordered_set<core::SymbolId> moduleValues;
    for (const auto& c : p.constants) moduleValues.insert(c.symbol);
    for (const auto& parameter : p.parameters) moduleValues.insert(parameter.symbol);
    for (const auto& e : p.enums) for (const auto& item : e.items) moduleValues.insert(item.symbol);
    for (const auto& s : p.structs) {
        auto available = moduleValues;
        if (!validateMembersDependencies(p, s.members, available, s.name, err)) return false;
        for (const auto& req : s.requirements) {
            if (!req) { err = s.name + ": null requires expression"; return false; }
            if (req->type != core::ExprType::Boolean) { err = s.name + ": requires expression must be boolean"; return false; }
            if (!validateExprDependencies(p, req.get(), available, s.name + ".requires", err)) return false;
        }
    }
    return true;
}

bool fixedMembersExtent(const Module& p, const std::vector<std::unique_ptr<Op>>& members,
                        runtime::LayoutSize& extent, std::string& err) {
    runtime::LayoutSize cursor = 0;
    runtime::LayoutSize maxEnd = 0;
    for (const auto& op : members) {
        if (!op) { err = "null plan operation"; return false; }
        runtime::LayoutSize end = cursor;
        switch (op->kind) {
        case OpKind::Virtual: break;
        case OpKind::Alias: break;
        case OpKind::Field: {
            const auto& f = static_cast<const Field&>(*op);
            auto fs = fixedTypeSize(p, f.type, err);
            if (!fs) return false;
            runtime::LayoutSize n = *fs;
            if (!addChecked(cursor, n, end)) { err = "fixed layout overflow"; return false; }
            break;
        }
        case OpKind::Bits:
            if (!addChecked(cursor, static_cast<const Bits&>(*op).storageBytes, end)) {
                err = "fixed layout overflow"; return false;
            }
            break;
        case OpKind::Align: {
            const auto& a = static_cast<const Align&>(*op);
            if (!a.staticAlignment || *a.staticAlignment == 0) return false;
            runtime::LayoutSize rem = cursor % *a.staticAlignment;
            if (rem && !addChecked(cursor, *a.staticAlignment - rem, end)) {
                err = "fixed layout overflow"; return false;
            }
            break;
        }
        case OpKind::Block: {
            const auto& b = static_cast<const Block&>(*op);
            if (!b.staticSize || !addChecked(cursor, *b.staticSize, end)) {
                err = "block has no fixed size"; return false;
            }
            break;
        }
        case OpKind::At: {
            const auto& a = static_cast<const At&>(*op);
            if (!a.staticOffset) return false;
            runtime::LayoutSize inner = 0;
            if (!fixedMembersExtent(p, a.members, inner, err)) return false;
            if (*a.staticOffset > std::numeric_limits<runtime::LayoutSize>::max() - inner) {
                err = "fixed layout overflow"; return false;
            }
            end = std::max(cursor, *a.staticOffset + inner);
            break;
        }
        case OpKind::Conditional: {
            const auto& c = static_cast<const Conditional&>(*op);
            runtime::LayoutSize thenSize = 0, elseSize = 0;
            if (!fixedMembersExtent(p, c.thenMembers, thenSize, err)) return false;
            if (!c.elseMembers.empty() && !fixedMembersExtent(p, c.elseMembers, elseSize, err)) return false;
            if (thenSize != elseSize) return false;
            if (!addChecked(cursor, thenSize, end)) { err = "fixed layout overflow"; return false; }
            break;
        }
        case OpKind::Variant:
        case OpKind::Callback:
            return false;
        }
        cursor = (op->kind == OpKind::At) ? cursor : end;
        maxEnd = std::max(maxEnd, end);
    }
    extent = maxEnd;
    return true;
}

bool computeStructLayout(Module& p, Struct& s, std::string& err) {
    runtime::LayoutSize offset = 0;
    runtime::LayoutSize maxEnd = 0;
    bool known = true;

    for (auto& op : s.members) {
        if (!op) {
            err = "null plan operation in struct: " + s.name;
            return false;
        }

        if (op->kind == OpKind::Field) {
            auto* f = static_cast<Field*>(op.get());
            if (known) f->staticOffset = offset;

            std::string e;
            auto fs = fixedTypeSize(p, f->type, e);
            if (!fs || (!f->staticLength &&
                (f->type.name == "bytes" || f->type.name == "string") &&
                !f->type.dimensions.empty())) {
                known = false;
                continue;
            }
            runtime::LayoutSize n = *fs;
            runtime::LayoutSize end = 0;
            if (!addChecked(offset, n, end)) {
                err = "struct layout overflow: " + s.name;
                return false;
            }
            offset = end;
            maxEnd = std::max(maxEnd, end);
            f->staticSize = n;
        }
        else if (op->kind == OpKind::Bits) {
            auto* b = static_cast<Bits*>(op.get());
            if (known) {
                runtime::LayoutSize end = 0;
                if (!addChecked(offset, b->storageBytes, end)) {
                    err = "struct layout overflow: " + s.name;
                    return false;
                }
                offset = end;
                maxEnd = std::max(maxEnd, end);
            }
        }
        else if (op->kind == OpKind::Align) {
            auto* a = static_cast<Align*>(op.get());
            if (known && a->staticAlignment) {
                runtime::LayoutSize rem = offset % *a->staticAlignment;
                if (rem) {
                    runtime::LayoutSize end = 0;
                    if (!addChecked(offset, *a->staticAlignment - rem, end)) {
                        err = "struct layout overflow: " + s.name;
                        return false;
                    }
                    offset = end;
                }
                maxEnd = std::max(maxEnd, offset);
            } else {
                known = false;
            }
        }
        else if (op->kind == OpKind::Block) {
            auto* b = static_cast<Block*>(op.get());
            if (known && b->staticSize) {
                runtime::LayoutSize end = 0;
                if (!addChecked(offset, *b->staticSize, end)) {
                    err = "struct layout overflow: " + s.name;
                    return false;
                }
                offset = end;
                maxEnd = std::max(maxEnd, end);
            } else {
                known = false;
            }
        }
        else if (op->kind == OpKind::At) {
            // `at(offset)` places members at an absolute offset but does not
            // advance the sequential layout cursor. The structure's extent
            // is the maximum of the sequential cursor and the absolute range.
            if (known) {
                auto* a = static_cast<At*>(op.get());
                if (!a->staticOffset && a->offset) {
                    runtime::SymbolEnvironment nextEnv;
                    nextEnv.emplace(core::BuiltinNextSymbolId, runtime::Value(offset));
                    runtime::LayoutSize resolved = 0;
                    std::string evalErr;
                    if (runtime::evaluateSize(a->offset.get(), nextEnv, resolved, evalErr)) a->staticOffset = resolved;
                }
                runtime::LayoutSize inner = 0;
                std::string e;
                if (!a->staticOffset || !fixedMembersExtent(p, a->members, inner, e) ||
                    *a->staticOffset > std::numeric_limits<runtime::LayoutSize>::max() - inner) {
                    known = false;
                } else {
                    maxEnd = std::max(maxEnd, *a->staticOffset + inner);
                }
            }
        }
        else if (op->kind == OpKind::Alias) {
            // Read-only aliases consume no layout space.
        }
        else if (op->kind == OpKind::Variant || op->kind == OpKind::Callback) {
            known = false;
        }
    }

    if (known) s.staticSize = maxEnd;
    return true;
}

} // namespace


bool multiplyBounds(const LayoutBounds& in, runtime::LayoutSize n,
                    LayoutBounds& out, std::string& err) {
    if (in.minSize && n > std::numeric_limits<runtime::LayoutSize>::max() / in.minSize) {
        err = "layout minimum overflow"; return false;
    }
    out.minSize = in.minSize * n;
    if (in.maxSize) {
        if (*in.maxSize && n > std::numeric_limits<runtime::LayoutSize>::max() / *in.maxSize) {
            err = "layout maximum overflow"; return false;
        }
        out.maxSize = *in.maxSize * n;
    }
    return true;
}

bool dimensionCount(const core::Dimension& d, runtime::LayoutSize& n, bool& exact,
                    std::string& err) {
    // Dynamic and remaining dimensions are runtime-sized by definition.
    // Their expressions may be semantically valid and executable, but they
    // cannot establish a finite compile-time bound here.
    if (d.kind == core::Dimension::Kind::Dynamic ||
        d.kind == core::Dimension::Kind::Remaining) {
        exact = false;
        return true;
    }
    if (!d.expression) { err = "layout dimension has no expression"; return false; }
    std::string e;
    if (runtime::evaluateSize(d.expression.get(), runtime::SymbolEnvironment{}, n, e)) {
        exact = true;
        return true;
    }
    if (!e.empty() && e.rfind("unknown identifier:", 0) != 0 && e.rfind("unknown symbol:", 0) != 0) {
        err = e; return false;
    }
    exact = false;
    return true;
}

struct SequenceBounds {
    runtime::LayoutSize cursorMin = 0;
    std::optional<runtime::LayoutSize> cursorMax = runtime::LayoutSize(0);
    runtime::LayoutSize extentMin = 0;
    std::optional<runtime::LayoutSize> extentMax = runtime::LayoutSize(0);
};

bool addCursor(SequenceBounds& s, const LayoutBounds& b, std::string& err) {
    runtime::LayoutSize nextMin = 0;
    if (!addChecked(s.cursorMin, b.minSize, nextMin)) {
        err = "layout minimum overflow";
        return false;
    }
    s.cursorMin = nextMin;

    if (s.cursorMax && b.maxSize) {
        runtime::LayoutSize nextMax = 0;
        if (!addChecked(*s.cursorMax, *b.maxSize, nextMax)) {
            err = "layout maximum overflow";
            return false;
        }
        s.cursorMax = nextMax;
    } else {
        s.cursorMax.reset();
    }

    s.extentMin = std::max(s.extentMin, s.cursorMin);
    if (s.extentMax && s.cursorMax) {
        *s.extentMax = std::max(*s.extentMax, *s.cursorMax);
    } else {
        s.extentMax.reset();
    }
    return true;
}

bool mergeExtent(SequenceBounds& s, const SequenceBounds& child) {
    s.extentMin = std::max(s.extentMin, child.extentMin);
    if (s.extentMax && child.extentMax) {
        *s.extentMax = std::max(*s.extentMax, *child.extentMax);
    } else {
        s.extentMax.reset();
    }
    return true;
}

LayoutBounds layoutBoundsImpl(const Module& p, const Type& t,
                              std::unordered_set<core::SymbolId>& visiting,
                              std::string& err);

bool sequenceBounds(const Module& p, const std::vector<std::unique_ptr<Op>>& members,
                    runtime::LayoutSize initialCursor,
                    std::unordered_set<core::SymbolId>& visiting,
                    SequenceBounds& out, std::string& err);

LayoutBounds memberBounds(const Module& p, const Op& op,
                          std::unordered_set<core::SymbolId>& visiting,
                          std::string& err) {
    switch (op.kind) {
    case OpKind::Virtual:
    case OpKind::Alias:
        return LayoutBounds{0, 0};
    case OpKind::Field: {
        const auto& f = static_cast<const Field&>(op);
        return layoutBoundsImpl(p, f.type, visiting, err);
    }
    case OpKind::Bits: {
        const auto& b = static_cast<const Bits&>(op);
        return LayoutBounds{b.storageBytes, b.storageBytes};
    }
    case OpKind::Block: {
        const auto& b = static_cast<const Block&>(op);
        if (b.staticSize) return LayoutBounds{*b.staticSize, *b.staticSize};
        return LayoutBounds{0, std::nullopt};
    }
    case OpKind::Align:
        return LayoutBounds{0, 0};
    case OpKind::At: {
        const auto& a = static_cast<const At&>(op);
        if (!a.staticOffset) return LayoutBounds{0, std::nullopt};

        SequenceBounds inner;
        if (!sequenceBounds(p, a.members, *a.staticOffset, visiting, inner, err)) return {};
        return LayoutBounds{inner.extentMin, inner.extentMax};
    }
    case OpKind::Conditional: {
        const auto& c = static_cast<const Conditional&>(op);
        auto branch = [&](const std::vector<std::unique_ptr<Op>>& members) -> LayoutBounds {
            SequenceBounds seq;
            if (!sequenceBounds(p, members, 0, visiting, seq, err)) return {};
            return LayoutBounds{seq.extentMin, seq.extentMax};
        };
        LayoutBounds thenB = branch(c.thenMembers);
        if (!err.empty()) return {};
        LayoutBounds elseB = c.elseMembers.empty() ? LayoutBounds{0, 0} : branch(c.elseMembers);
        if (!err.empty()) return {};
        LayoutBounds result;
        result.minSize = std::min(thenB.minSize, elseB.minSize);
        if (thenB.maxSize && elseB.maxSize) result.maxSize = std::max(*thenB.maxSize, *elseB.maxSize);
        else result.maxSize.reset();
        return result;
    }
    case OpKind::Variant: {
        const auto& v = static_cast<const Variant&>(op);
        bool have = false;
        runtime::LayoutSize minValue = std::numeric_limits<runtime::LayoutSize>::max();
        std::optional<runtime::LayoutSize> maxValue = runtime::LayoutSize(0);
        auto take = [&](const LayoutBounds& b) {
            have = true;
            minValue = std::min(minValue, b.minSize);
            if (maxValue && b.maxSize) *maxValue = std::max(*maxValue, *b.maxSize);
            else maxValue.reset();
        };
        auto branchBounds = [&](const VariantCase& c) -> LayoutBounds {
            if (c.type) return layoutBoundsImpl(p, *c.type, visiting, err);
            SequenceBounds branch;
            if (!sequenceBounds(p, c.members, 0, visiting, branch, err)) return {};
            return LayoutBounds{branch.extentMin, branch.extentMax};
        };
        for (const auto& c : v.cases) {
            auto b = branchBounds(c);
            if (!err.empty()) return {};
            take(b);
        }
        if (v.hasDefault) {
            if (v.defaultType) {
                auto b = layoutBoundsImpl(p, *v.defaultType, visiting, err);
                if (!err.empty()) return {};
                take(b);
            } else {
                SequenceBounds branch;
                if (!sequenceBounds(p, v.defaultMembers, 0, visiting, branch, err)) return {};
                take(LayoutBounds{branch.extentMin, branch.extentMax});
            }
        }
        if (!have) return LayoutBounds{0, 0};
        return LayoutBounds{minValue, maxValue};
    }
    case OpKind::Callback:
        return LayoutBounds{0, 0};
    }
    err = "unknown plan operation";
    return {};
}

bool sequenceBounds(const Module& p, const std::vector<std::unique_ptr<Op>>& members,
                    runtime::LayoutSize initialCursor,
                    std::unordered_set<core::SymbolId>& visiting,
                    SequenceBounds& out, std::string& err) {
    SequenceBounds seq;
    seq.cursorMin = initialCursor;
    seq.cursorMax = initialCursor;
    seq.extentMin = initialCursor;
    seq.extentMax = initialCursor;

    for (const auto& op : members) {
        if (!op) { err = "null plan operation"; return false; }

        if (op->kind == OpKind::At) {
            const auto& a = static_cast<const At&>(*op);
            if (!a.staticOffset) {
                seq.extentMax.reset();
                continue;
            }
            SequenceBounds inner;
            if (!sequenceBounds(p, a.members, *a.staticOffset, visiting, inner, err)) return false;
            mergeExtent(seq, inner);
            continue; // absolute placement does not advance the sequential cursor
        }

        if (op->kind == OpKind::Align) {
            const auto& a = static_cast<const Align&>(*op);
            if (!a.staticAlignment || *a.staticAlignment == 0) {
                err = "align is not finalized";
                return false;
            }
            const auto alignUp = [&](runtime::LayoutSize x, runtime::LayoutSize& aligned) {
                const runtime::LayoutSize rem = x % *a.staticAlignment;
                return rem == 0 ? (aligned = x, true)
                                : addChecked(x, *a.staticAlignment - rem, aligned);
            };
            runtime::LayoutSize minAligned = 0;
            if (!alignUp(seq.cursorMin, minAligned)) { err = "layout minimum overflow"; return false; }
            seq.cursorMin = minAligned;
            if (seq.cursorMax) {
                runtime::LayoutSize maxAligned = 0;
                if (!alignUp(*seq.cursorMax, maxAligned)) { err = "layout maximum overflow"; return false; }
                seq.cursorMax = maxAligned;
            }
            seq.extentMin = std::max(seq.extentMin, seq.cursorMin);
            if (seq.extentMax && seq.cursorMax) *seq.extentMax = std::max(*seq.extentMax, *seq.cursorMax);
            else seq.extentMax.reset();
            continue;
        }

        auto b = memberBounds(p, *op, visiting, err);
        if (!err.empty() || !addCursor(seq, b, err)) return false;
    }

    out = std::move(seq);
    return true;
}

LayoutBounds layoutBoundsImpl(const Module& p, const Type& t,
                              std::unordered_set<core::SymbolId>& visiting,
                              std::string& err) {
    LayoutBounds base;
    if (!t.terminator.empty()) {
        if (t.kind != core::TypeKind::Bytes || !t.maxPayload.has_value() || t.terminator.empty()) { err = "invalid terminated sequence type"; return {}; }
        if (*t.maxPayload > std::numeric_limits<runtime::LayoutSize>::max() - static_cast<runtime::LayoutSize>(t.terminator.size())) { err = "terminated sequence maximum length overflow"; return {}; }
        base.minSize = static_cast<runtime::LayoutSize>(t.terminator.size());
        base.maxSize = *t.maxPayload + static_cast<runtime::LayoutSize>(t.terminator.size());
    } else if (auto s = primitiveSize(t.name)) base = LayoutBounds{*s, *s};
    else if (t.name == "bytes" || t.name == "string") base = LayoutBounds{1, 1};
    else if (const auto* sp = findStructByType(p, t)) {
        if (!t.reference.valid()) { err = "named type has no SymbolId: " + t.name; return {}; }
        if (!visiting.insert(t.reference.id).second) { err = "cyclic struct reference in layout bounds: " + t.name; return {}; }
        SequenceBounds seq;
        if (!sequenceBounds(p, sp->members, 0, visiting, seq, err)) {
            visiting.erase(t.reference.id);
            return {};
        }
        visiting.erase(t.reference.id);
        base = LayoutBounds{seq.extentMin, seq.extentMax};
    } else {
        const auto ei = t.reference.valid() ? p.enumIndexBySymbol.find(t.reference.id) : p.enumIndexBySymbol.end();
        if (ei != p.enumIndexBySymbol.end()) {
            const auto& e = p.enums[ei->second];
            std::string underlying;
            std::unordered_set<core::SymbolId> enumVisiting;
            if (!resolveIntegerPrimitive(p, e.underlying, underlying, enumVisiting, err)) return {};
            const auto sz = primitiveSize(underlying);
            if (!sz) { err = "invalid enum underlying type in plan: " + e.name; return {}; }
            base = LayoutBounds{*sz, *sz};
        } else {
            const auto* ap = findAliasByType(p, t);
            if (!ap) { err = "unknown named type in plan: " + t.name; return {}; }
            if (!t.reference.valid()) { err = "named type has no SymbolId: " + t.name; return {}; }
            if (!visiting.insert(t.reference.id).second) { err = "cyclic alias in layout bounds: " + t.name; return {}; }
            base = layoutBoundsImpl(p, ap->target, visiting, err);
            visiting.erase(t.reference.id);
            if (!err.empty()) return {};
        }
    }

    for (const auto& d : t.dimensions) {
        if (d.kind == core::Dimension::Kind::Remaining) {
            base.minSize = 0;
            base.maxSize.reset();
            continue;
        }
        runtime::LayoutSize n = 0;
        bool exact = false;
        if (!dimensionCount(d, n, exact, err)) return {};
        if (exact) {
            LayoutBounds next;
            if (!multiplyBounds(base, n, next, err)) return {};
            base = next;
        } else {
            base.minSize = 0;
            base.maxSize.reset();
        }
    }
    return base;
}

std::optional<runtime::LayoutSize> fixedTypeSize(const Module& p,const Type& t,std::string& error){std::unordered_set<core::SymbolId> v;runtime::LayoutSize n=0;if(!fixedTypeSizeImpl(p,t,n,v,error))return std::nullopt;return n;}

LayoutBounds layoutBounds(const Module& p, const Type& t, std::string& error) {
    error.clear();
    std::unordered_set<core::SymbolId> visiting;
    return layoutBoundsImpl(p, t, visiting, error);
}

std::optional<runtime::LayoutSize> sizeInBytes(const Module& p, const Type& t, std::string& error) { auto b=layoutBounds(p,t,error); if(!error.empty() || !b.exact()) { if(error.empty()) error="size_in_bytes is not statically exact"; return std::nullopt; } return b.minSize; }
std::optional<runtime::LayoutSize> minSizeInBytes(const Module& p, const Type& t, std::string& error) { auto b=layoutBounds(p,t,error); if(!error.empty()) return std::nullopt; return b.minSize; }
std::optional<runtime::LayoutSize> maxSizeInBytes(const Module& p, const Type& t, std::string& error) { auto b=layoutBounds(p,t,error); if(!error.empty() || !b.maxSize) { if(error.empty()) error="max_size_in_bytes is unbounded"; return std::nullopt; } return *b.maxSize; }

std::unique_ptr<Module> build(const ir::Module& src,std::string& error){
    error.clear();
    if (!validateIRIdentity(src, error)) return {};
    auto p=std::make_unique<Module>();
    p->documentation=src.documentation;
    p->attributeDefinitions=src.attributeDefinitions;
    p->endian=resolveEndian(core::Endian::Inherit,src.endian,core::Endian::Inherit);
    p->symbolTable = src.symbolTable;
    for (const auto& a : src.aliases) {
        if (a.symbol == core::InvalidSymbolId) { error = "alias has invalid SymbolId: " + a.name; return {}; }
        const auto* sym = src.symbolTable.find(a.symbol);
        if (!sym || sym->kind != core::SymbolKind::TypeAlias) { error = "alias SymbolId has wrong kind: " + a.name; return {}; }
    }
    for (const auto& s : src.structs) {
        if (!s) { error = "null IR struct"; return {}; }
        if (s->symbol == core::InvalidSymbolId) { error = "struct has invalid SymbolId: " + s->name; return {}; }
        const auto* sym = src.symbolTable.find(s->symbol);
        if (!sym || sym->kind != core::SymbolKind::Struct) { error = "struct SymbolId has wrong kind: " + s->name; return {}; }
    }
    for (const auto& e : src.enums) {
        if (e.symbol == core::InvalidSymbolId) { error = "enum has invalid SymbolId: " + e.name; return {}; }
        const auto* sym = src.symbolTable.find(e.symbol);
        if (!sym || sym->kind != core::SymbolKind::Enum) { error = "enum SymbolId has wrong kind: " + e.name; return {}; }
        for (const auto& item : e.items) {
            if (item.symbol == core::InvalidSymbolId) { error = "enum item has invalid SymbolId: " + e.name + "::" + item.name; return {}; }
            const auto* is = src.symbolTable.find(item.symbol);
            if (!is || is->kind != core::SymbolKind::EnumItem) { error = "enum item SymbolId has wrong kind: " + e.name + "::" + item.name; return {}; }
        }
    }
    for (const auto& c : src.constants) {
        if (c.symbol == core::InvalidSymbolId) { error = "constant has invalid SymbolId: " + c.name; return {}; }
        const auto* sym = src.symbolTable.find(c.symbol);
        if (!sym || sym->kind != core::SymbolKind::Constant) { error = "constant SymbolId has wrong kind: " + c.name; return {}; }
    }
    for (const auto& c : src.callbacks) {
        if (c.symbol == core::InvalidSymbolId) { error = "callback has invalid SymbolId: " + c.name; return {}; }
        const auto* sym = src.symbolTable.find(c.symbol);
        if (!sym || sym->kind != core::SymbolKind::Callback) { error = "callback SymbolId has wrong kind: " + c.name; return {}; }
    }
    for (const auto& x : src.parameters) {
        if (x.symbol == core::InvalidSymbolId) { error = "parameter has invalid SymbolId: " + x.name; return {}; }
        const auto* sym = src.symbolTable.find(x.symbol);
        if (!sym || sym->kind != core::SymbolKind::Parameter) { error = "parameter SymbolId has wrong kind: " + x.name; return {}; }
        if (x.type.kind != core::TypeKind::Primitive || x.type.dimensions.size() != 0) { error = "parameter type is not scalar: " + x.name; return {}; }
    }
    for(const auto& a:src.aliases){Alias x;x.symbol=a.symbol;x.attributes=a.attributes;x.documentation=a.documentation;x.name=a.name;x.target=a.target;p->aliasIndexBySymbol[x.symbol]=p->aliases.size();p->aliases.push_back(std::move(x));}
    for(const auto& s:src.structs){if(!s){error="null IR struct";return{};}Struct x;x.symbol=s->symbol;x.attributes=s->attributes;x.documentation=s->documentation;x.name=s->name;x.endian=resolveEndian(s->endian,src.endian,src.endian);p->structIndexBySymbol[x.symbol]=p->structs.size();p->structs.push_back(std::move(x));}
    for(const auto& e:src.enums){Enum x;x.symbol=e.symbol;x.attributes=e.attributes;x.documentation=e.documentation;x.name=e.name;x.underlying=e.underlying;if(x.underlying.name.empty()){x.underlying.kind=core::TypeKind::Primitive;x.underlying.name="u32";}p->enumIndexBySymbol[x.symbol]=p->enums.size();p->enums.push_back(std::move(x));}
    for(const auto& c:src.callbacks){CallbackDecl x;x.symbol=c.symbol;x.attributes=c.attributes;x.documentation=c.documentation;x.name=c.name;x.direction=c.direction;x.hasParameter=c.hasParameter;x.parameter=c.parameter;p->callbacks.push_back(std::move(x));}
    for(const auto& x:src.parameters){Parameter q;q.symbol=x.symbol;q.attributes=x.attributes;q.documentation=x.documentation;q.name=x.name;q.type=x.type;p->parameterIndexBySymbol[q.symbol]=p->parameters.size();p->parameters.push_back(std::move(q));}
    runtime::SymbolEnvironment env;

    // Materialize module constants strictly by SymbolId. Forward references are
    // allowed only between constants and are resolved by dependency identity,
    // never by spelling.
    std::unordered_map<core::SymbolId, const ir::Constant*> constants;
    std::unordered_set<core::SymbolId> pending;
    for (const auto& c : src.constants) {
        if (c.symbol == core::InvalidSymbolId) { error = "constant has invalid SymbolId: " + c.name; return {}; }
        constants.emplace(c.symbol, &c);
        pending.insert(c.symbol);
    }
    while (!pending.empty()) {
        bool progress = false;
        std::vector<core::SymbolId> done;
        for (const auto id : pending) {
            const auto* c = constants.at(id);
            std::unordered_set<core::SymbolId> refs;
            collectExprSymbols(c->expr.get(), refs, error);
            if (!error.empty()) { error = "constant " + c->name + ": " + error; return {}; }
            bool waiting = false;
            for (const auto dep : refs) {
                if (pending.count(dep)) { waiting = true; continue; }
                if (!env.count(dep)) {
                    const auto* sym = src.symbolTable.find(dep);
                    if (!sym || sym->kind != core::SymbolKind::Constant) {
                        error = "constant " + c->name + ": references unavailable symbol: " + (sym ? sym->name : std::to_string(dep));
                        return {};
                    }
                }
            }
            if (waiting) continue;
            runtime::Value v;
            if (!evaluateExpr(c->expr.get(), env, v, error)) {
                error = "constant " + c->name + ": " + error;
                return {};
            }
            env[id] = v;
            done.push_back(id);
            progress = true;
        }
        for (const auto id : done) pending.erase(id);
        if (!progress) {
            error = "constant dependency cycle or unresolved forward reference";
            return {};
        }
    }

    for (const auto& ref : src.order) {
        if (ref.kind != ir::TopLevelRef::Kind::Const) continue;
        if (ref.index >= src.constants.size()) { error = "invalid constant order index"; return {}; }
        const auto& c = src.constants[ref.index];
        const auto it = env.find(c.symbol);
        if (it == env.end()) { error = "internal constant resolution failure: " + c.name; return {}; }
        Constant pc; pc.symbol=c.symbol; pc.attributes=c.attributes; pc.documentation=c.documentation; pc.name=c.name; pc.value=it->second; pc.computed=c.computed; p->constantIndexBySymbol[pc.symbol]=p->constants.size(); p->constants.push_back(std::move(pc));
    }

    // Enum values use the same SymbolEnvironment. An enum item can reference
    // module constants or an earlier item in the same enum; later items are not
    // visible because their SymbolId is not yet materialized.
    for (const auto& ref : src.order) {
        if (ref.kind != ir::TopLevelRef::Kind::Enum) continue;
        if (ref.index >= src.enums.size()) { error = "invalid enum order index"; return {}; }
        const auto& e = src.enums[ref.index];
        auto& pe = p->enums[ref.index];
        std::string underlying;
        if (pe.underlying.name.empty()) underlying = "u32";
        else {
            std::unordered_set<core::SymbolId> visiting;
            if (!resolveIntegerPrimitive(*p, pe.underlying, underlying, visiting, error)) {
                error = "enum " + e.name + ": " + error;
                return {};
            }
        }
        std::set<uint64_t> seenValues;
        const bool signedUnderlying = !underlying.empty() && underlying[0]=='i';
        for (const auto& item : e.items) {
            if (item.symbol == core::InvalidSymbolId) { error = "enum " + e.name + " item has invalid SymbolId: " + item.name; return {}; }
            std::unordered_set<core::SymbolId> refs;
            collectExprSymbols(item.value.get(), refs, error);
            if (!error.empty()) { error = "enum " + e.name + " item " + item.name + ": " + error; return {}; }
            for (const auto dep : refs) {
                if (!env.count(dep)) {
                    const auto* sym = src.symbolTable.find(dep);
                    error = "enum " + e.name + " item " + item.name + ": unavailable value symbol: " + (sym ? sym->name : std::to_string(dep));
                    return {};
                }
            }
            runtime::Value v;
            if (!evaluateExpr(item.value.get(), env, v, error)) {
                error = "enum " + e.name + " item " + item.name + ": " + error;
                return {};
            }
            std::string valueError;
            if (!enumValueFits(v, underlying, valueError)) {
                error = "enum " + e.name + " item " + item.name + ": " + valueError;
                return {};
            }
            uint64_t key = signedUnderlying ? static_cast<uint64_t>(std::get<int64_t>(v))
                                            : (std::holds_alternative<int64_t>(v) ? static_cast<uint64_t>(std::get<int64_t>(v)) : std::get<uint64_t>(v));
            if (!seenValues.insert(key).second) { error = "enum " + e.name + ": duplicate numeric value for item " + item.name; return {}; }
            pe.values[item.name] = v;
            pe.items.push_back({item.symbol, item.name, v, item.documentation});
            env[item.symbol] = v;
        }
    }

    for (const auto& c : src.constants) {
        if (!validateExpressionType(src, c.expr.get(), env, "constant " + c.name, error)) return {};
    }
    for (const auto& e : src.enums) for (const auto& item : e.items) {
        if (!validateExpressionType(src, item.value.get(), env, "enum " + e.name + "::" + item.name, error)) return {};
    }

    for(size_t i=0;i<src.structs.size();++i){
        for (const auto& req : src.structs[i]->requirements) p->structs[i].requirements.push_back(core::cloneExpr(req.get()));
        if(!lowerMembers(src.structs[i]->members,src,src.structs[i]->endian,src.endian,env,false,p->structs[i].members,error)){error="in struct "+src.structs[i]->name+": "+error;return{};}
        if(!computeStructLayout(*p,p->structs[i],error)) return{};
    }
    for (auto& st : p->structs) {
        Type self; self.kind=core::TypeKind::Named; self.name=st.name; self.reference.id=st.symbol;
        std::string boundsErr;
        const auto bounds = layoutBounds(*p, self, boundsErr);
        if (!boundsErr.empty()) { error = "in struct " + st.name + ": " + boundsErr; return {}; }
        st.minSizeInBytes = bounds.minSize;
        st.maxSizeInBytes = bounds.maxSize;
        if (bounds.exact()) st.sizeInBytes = bounds.minSize;
    }
    if (!validateLayoutDependencies(*p, error)) return {};

    // A Plan is the executable contract. Do not publish a plan until its
    // explicit layout dependency graph and statically decidable safety
    // invariants have both been validated.
    LayoutGraph graph;
    if (!buildLayoutGraph(*p, graph, error)) return {};
    if (!validateStaticLayoutSafety(*p, error)) return {};
    if (!validatePlanExpressionTypes(src, *p, env, error)) { error = "invalid expression typing: " + error; return {}; }
    if (!validateExecutablePlan(*p, error)) { error = "invalid executable plan: " + error; return {}; }

    return p;
}

} // namespace embx::plan
