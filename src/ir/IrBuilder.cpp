#include "ir/IrBuilder.h"
#include "core/Resolver.h"

#include <set>
#include <sstream>
#include <unordered_map>
#include <utility>

namespace embx::ir {
namespace {

core::Attribute canonicalAttribute(const ast::Attribute& a) {
    core::Attribute r; r.name=a.name; r.value=a.value; r.hasValue=a.hasValue;
    return r;
}
std::vector<core::Attribute> canonicalAttributes(const std::vector<ast::Attribute>& a) {
    std::vector<core::Attribute> r; r.reserve(a.size());
    for (const auto& x : a) r.push_back(canonicalAttribute(x));
    return r;
}
core::Endian canonicalEndian(ast::Endian e) { return static_cast<core::Endian>(e); }
core::CallbackDirection canonicalDirection(ast::CallbackDirection d) { return static_cast<core::CallbackDirection>(d); }


struct ResolutionContext {
    const core::NameResolver* resolver = nullptr;
    core::Scope* scope = nullptr;
    const std::string* currentFieldName = nullptr;

    core::SymbolId resolve(const std::string& name) const noexcept {
        // A field is not in scope for its own declaration. Later sibling
        // fields remain resolvable by SymbolId so PlanBuilder can diagnose
        // declaration-order dependencies explicitly.
        if (currentFieldName && *currentFieldName == name)
            return core::InvalidSymbolId;
        return resolver ? resolver->resolve(name) : core::InvalidSymbolId;
    }
};

std::unique_ptr<core::Expr> lowerExpr(const ast::Expr* e, const ResolutionContext& ctx, std::string& err) {
    if (!e) return {};
    auto r = std::make_unique<core::Expr>();
    r->kind = static_cast<core::ExprKind>(e->kind);
    r->text = e->text;
    r->op = e->op;
    r->type = e->type;
    if (e->kind == ast::ExprKind::Identifier) {
        if (e->text == "$next") r->reference.id = core::BuiltinNextSymbolId;
        else if (e->text == "$size_in_bytes") r->reference.id = core::BuiltinSizeInBytesSymbolId;
        else if (e->text == "$min_size_in_bytes") r->reference.id = core::BuiltinMinSizeInBytesSymbolId;
        else if (e->text == "$max_size_in_bytes") r->reference.id = core::BuiltinMaxSizeInBytesSymbolId;
        else r->reference.id = ctx.resolve(e->text);
    }
    r->left = lowerExpr(e->left.get(), ctx, err);
    if (e->left && !r->left) return {};
    r->right = lowerExpr(e->right.get(), ctx, err);
    if (e->right && !r->right) return {};
    std::string typeError;
    if (!core::annotateExprTypes(r.get(), typeError)) { err = typeError; return {}; }
    return r;
}

Type lowerType(const ast::TypeRef& t, const ResolutionContext& ctx, std::string& err) {
    Type r; r.name=t.name; r.terminator=t.terminator; r.maxPayload=t.maxPayload; r.dimensions.reserve(t.suffixes.size());
    for(const auto& s:t.suffixes){
        if(s.dynamic) r.dimensions.push_back(s.expr ? core::Dimension::dynamic(lowerExpr(s.expr.get(),ctx,err)) : core::Dimension::remaining());
        else r.dimensions.push_back(core::Dimension::fixed(lowerExpr(s.expr.get(),ctx,err)));
        if(!err.empty()) return r;
    }
    if(t.name=="bytes") r.kind=TypeKind::Bytes;
    else if(t.name=="string") r.kind=TypeKind::String;
    else if(!t.name.empty() && ((t.name[0]=='u'||t.name[0]=='i')||t.name=="f32"||t.name=="f64")) r.kind=TypeKind::Primitive;
    else { r.kind=TypeKind::Named; r.reference.id=ctx.resolve(t.name); }
    return r;
}

bool prepareScope(const std::vector<std::unique_ptr<ast::Member>>& members,
                  core::Scope& scope, core::SymbolTable& table, std::string& err) {
    for (const auto& m : members) {
        if (!m) { err = "null AST member"; return false; }
        if (const auto* v = dynamic_cast<const ast::Virtual*>(m.get())) {
            const auto id = table.declareScoped(core::SymbolKind::VirtualField, v->name);
            if (id == core::InvalidSymbolId || !scope.declare(v->name, id)) {
                err = "duplicate member: " + v->name;
                return false;
            }
        } else if (const auto* a = dynamic_cast<const ast::Alias*>(m.get())) {
            const auto id = table.declareScoped(core::SymbolKind::AliasField, a->name);
            if (id == core::InvalidSymbolId || !scope.declare(a->name, id)) { err = "duplicate field: " + a->name; return false; }
        } else if (const auto* f = dynamic_cast<const ast::Field*>(m.get())) {
            const auto id = table.declareScoped(core::SymbolKind::Field, f->name);
            if (id == core::InvalidSymbolId || !scope.declare(f->name, id)) {
                err = "duplicate field: " + f->name;
                return false;
            }
        }
    }
    return true;
}

std::unique_ptr<Member> lowerMember(const ast::Member* m, const ResolutionContext& parentCtx, core::SymbolTable& table, std::string& err) {
    if(!m){err="null AST member";return{};}
    if(auto* v=dynamic_cast<const ast::Virtual*>(m)){
        auto r=std::make_unique<Virtual>(); r->attributes=canonicalAttributes(v->attributes); r->documentation=v->documentation; r->name=v->name;
        r->symbol=parentCtx.scope ? parentCtx.scope->resolve(r->name) : core::InvalidSymbolId;
        if(r->symbol==core::InvalidSymbolId){err="internal: virtual field symbol not prepared: "+r->name;return{};}
        r->expression=lowerExpr(v->expression.get(),parentCtx,err);
        if(!err.empty()) return {};
        return r;
    }
    if(auto* a=dynamic_cast<const ast::Alias*>(m)){
        auto r=std::make_unique<FieldAlias>(); r->attributes=canonicalAttributes(a->attributes); r->documentation=a->documentation; r->name=a->name; r->targetName=a->target;
        if (!parentCtx.scope) { err="internal: missing alias field scope"; return {}; }
        r->symbol=parentCtx.scope->resolve(r->name); r->target=parentCtx.resolve(a->target);
        if(r->symbol==core::InvalidSymbolId){err="internal: alias field symbol not prepared: "+r->name;return{};}
        return r;
    }
    if(auto* f=dynamic_cast<const ast::Field*>(m)){
        auto r=std::make_unique<Field>();
        r->attributes=canonicalAttributes(f->attributes);r->documentation=f->documentation;r->name=f->name;
        ResolutionContext ctx = parentCtx;
        ctx.currentFieldName = &f->name;
        // Field symbols are predeclared for the whole lexical scope. This makes
        // references to later fields resolvable by SymbolId; declaration order is
        // enforced separately by PlanBuilder's dependency validation. A field
        // referring to itself is likewise rejected there because it is not yet
        // available at its own layout position.
        if (!ctx.scope) { err = "internal: missing field scope"; return {}; }
        r->symbol = ctx.scope->resolve(r->name);
        if (r->symbol == core::InvalidSymbolId) { err = "internal: field symbol not prepared: " + r->name; return {}; }
        r->type = lowerType(f->type, ctx, err);
        if (!err.empty()) return {};
        for(const auto& mod:f->modifiers) if(mod.kind==ast::ModifierKind::Length){ r->type.dimensions.push_back(core::Dimension::dynamic(lowerExpr(mod.expr.get(),ctx,err))); break; }
        if(!err.empty())return{};
        r->bits=f->bits;r->endian=canonicalEndian(f->endian);r->assertion=f->assertion; if(f->transform){ r->transform=std::make_unique<Transform>(); r->transform->name=f->transform->name; for(const auto& e:f->transform->arguments){ auto q=lowerExpr(e.get(),ctx,err); if(!err.empty()) return {}; r->transform->arguments.push_back(std::move(q)); }}
        return r;
    }
    if(auto* b=dynamic_cast<const ast::Bits*>(m)){
        auto r=std::make_unique<Bits>();r->attributes=canonicalAttributes(b->attributes);r->documentation=b->documentation;
        core::Scope child(parentCtx.scope);
        for (const auto& f : b->fields) {
            if (!f) { err = "null AST bit field"; return {}; }
            const auto id = table.declareScoped(core::SymbolKind::Field, f->name);
            if (id == core::InvalidSymbolId || !child.declare(f->name, id)) { err = "duplicate bit field: " + f->name; return {}; }
        }
        core::NameResolver nested(parentCtx.resolver->symbols(), &child);
        ResolutionContext ctx{&nested, &child};
        for(const auto& f:b->fields){auto q=lowerMember(f.get(),ctx,table,err);if(!q)return{};auto* x=dynamic_cast<Field*>(q.get());r->totalBits+=x->bits;r->fields.push_back(std::unique_ptr<Field>(static_cast<Field*>(q.release())));}return r;
    }
    if(auto* c=dynamic_cast<const ast::Conditional*>(m)){
        auto r=std::make_unique<Conditional>(); r->attributes=canonicalAttributes(c->attributes); r->documentation=c->documentation;
        r->condition=lowerExpr(c->condition.get(),parentCtx,err); if(!err.empty()) return {};
        { core::Scope child(parentCtx.scope); if(!prepareScope(c->thenMembers,child,table,err)) return {}; core::NameResolver nested(parentCtx.resolver->symbols(),&child); ResolutionContext cc{&nested,&child}; for(const auto& z:c->thenMembers){auto q=lowerMember(z.get(),cc,table,err);if(!q)return{};r->thenMembers.push_back(std::move(q));} }
        { core::Scope child(parentCtx.scope); if(!prepareScope(c->elseMembers,child,table,err)) return {}; core::NameResolver nested(parentCtx.resolver->symbols(),&child); ResolutionContext cc{&nested,&child}; for(const auto& z:c->elseMembers){auto q=lowerMember(z.get(),cc,table,err);if(!q)return{};r->elseMembers.push_back(std::move(q));} }
        return r;
    }
    if(auto* v=dynamic_cast<const ast::Variant*>(m)){
        auto r=std::make_unique<Variant>();r->attributes=canonicalAttributes(v->attributes);r->documentation=v->documentation;r->name=v->name;r->discriminator=lowerExpr(v->discriminator.get(),parentCtx,err);if(!err.empty())return{};
        for(const auto& c:v->cases){VariantCase x;x.tag=lowerExpr(c.tag.get(),parentCtx,err);if(!err.empty())return{};if(c.type){x.hasType=true;x.type=lowerType(*c.type,parentCtx,err);if(!err.empty())return{};}else{core::Scope child(parentCtx.scope);if(!prepareScope(c.members, child, table, err))return{};core::NameResolver nested(parentCtx.resolver->symbols(), &child);
        ResolutionContext cc{&nested, &child};for(const auto& z:c.members){auto q=lowerMember(z.get(),cc,table,err);if(!q)return{};x.members.push_back(std::move(q));}}r->cases.push_back(std::move(x));}
        if(v->defaultType){r->hasDefault=true;r->defaultType=lowerType(*v->defaultType,parentCtx,err);if(!err.empty())return{};}else if(!v->defaultMembers.empty()){r->hasDefault=true;core::Scope child(parentCtx.scope);if(!prepareScope(v->defaultMembers, child, table, err))return{};core::NameResolver nested(parentCtx.resolver->symbols(), &child);
        ResolutionContext cc{&nested, &child};for(const auto& z:v->defaultMembers){auto q=lowerMember(z.get(),cc,table,err);if(!q)return{};r->defaultMembers.push_back(std::move(q));}}
        return r;
    }
    if(auto* b=dynamic_cast<const ast::Block*>(m)){auto r=std::make_unique<Block>();r->attributes=canonicalAttributes(b->attributes);r->documentation=b->documentation;r->name=b->name;r->size=lowerExpr(b->size.get(),parentCtx,err);if(!err.empty())return{};core::Scope child(parentCtx.scope);if(!prepareScope(b->members, child, table, err))return{};core::NameResolver nested(parentCtx.resolver->symbols(), &child);
        ResolutionContext cc{&nested, &child};for(const auto& z:b->members){auto q=lowerMember(z.get(),cc,table,err);if(!q)return{};r->members.push_back(std::move(q));}return r;}
    if(auto* a=dynamic_cast<const ast::At*>(m)){auto r=std::make_unique<At>();r->attributes=canonicalAttributes(a->attributes);r->documentation=a->documentation;r->offset=lowerExpr(a->offset.get(),parentCtx,err);if(!err.empty())return{};core::Scope child(parentCtx.scope);if(!prepareScope(a->members, child, table, err))return{};core::NameResolver nested(parentCtx.resolver->symbols(), &child);
        ResolutionContext cc{&nested, &child};for(const auto& z:a->members){auto q=lowerMember(z.get(),cc,table,err);if(!q)return{};r->members.push_back(std::move(q));}return r;}
    if(auto* a=dynamic_cast<const ast::Align*>(m)){auto r=std::make_unique<Align>();r->attributes=canonicalAttributes(a->attributes);r->documentation=a->documentation;r->alignment=lowerExpr(a->alignment.get(),parentCtx,err);return r;}
    if(auto* c=dynamic_cast<const ast::Callback*>(m)){auto r=std::make_unique<Callback>();r->attributes=canonicalAttributes(c->attributes);r->documentation=c->documentation;r->name=c->name;r->direction=canonicalDirection(c->direction);for(const auto& e:c->args){auto q=lowerExpr(e.get(),parentCtx,err);if(e&&!q)return{};r->args.push_back(std::move(q));}return r;}
    err="unknown AST member";return{};
}

bool duplicateFields(const std::vector<std::unique_ptr<Member>>& ms, std::string& err) {
    std::set<std::string> names;
    for (const auto& m : ms) {
        if (auto* f = dynamic_cast<Field*>(m.get())) {
            if (!names.insert(f->name).second) {
                err = "duplicate field: " + f->name;
                return false;
            }
        }
    }
    return true;
}

bool validOrder(const ast::Module& a, std::string& error) {
    const std::size_t total = a.aliases.size() + a.structs.size() + a.enums.size() +
                              a.constants.size() + a.callbacks.size() + a.parameters.size();
    if (a.order.size() != total) {
        std::ostringstream os;
        os << "invalid AST declaration order: expected " << total
           << " entries, got " << a.order.size();
        error = os.str();
        return false;
    }

    std::vector<bool> aliasSeen(a.aliases.size());
    std::vector<bool> structSeen(a.structs.size());
    std::vector<bool> enumSeen(a.enums.size());
    std::vector<bool> constSeen(a.constants.size());
    std::vector<bool> callbackSeen(a.callbacks.size());
    std::vector<bool> parameterSeen(a.parameters.size());

    for (const auto& ref : a.order) {
        bool ok = false;
        switch (ref.kind) {
        case ast::TopLevelRef::Kind::Alias:
            ok = ref.index < aliasSeen.size() && !aliasSeen[ref.index];
            if (ok) aliasSeen[ref.index] = true;
            break;
        case ast::TopLevelRef::Kind::Struct:
            ok = ref.index < structSeen.size() && !structSeen[ref.index];
            if (ok) structSeen[ref.index] = true;
            break;
        case ast::TopLevelRef::Kind::Enum:
            ok = ref.index < enumSeen.size() && !enumSeen[ref.index];
            if (ok) enumSeen[ref.index] = true;
            break;
        case ast::TopLevelRef::Kind::Const:
            ok = ref.index < constSeen.size() && !constSeen[ref.index];
            if (ok) constSeen[ref.index] = true;
            break;
        case ast::TopLevelRef::Kind::Callback:
            ok = ref.index < callbackSeen.size() && !callbackSeen[ref.index];
            if (ok) callbackSeen[ref.index] = true;
            break;
        case ast::TopLevelRef::Kind::Parameter:
            ok = ref.index < parameterSeen.size() && !parameterSeen[ref.index];
            if (ok) parameterSeen[ref.index] = true;
            break;
        }
        if (!ok) {
            error = "invalid AST declaration order reference";
            return false;
        }
    }
    return true;
}

core::SymbolKind symbolKind(const char* kind) {
    if (std::string(kind) == "alias") return core::SymbolKind::TypeAlias;
    if (std::string(kind) == "struct") return core::SymbolKind::Struct;
    if (std::string(kind) == "enum") return core::SymbolKind::Enum;
    if (std::string(kind) == "constant") return core::SymbolKind::Constant;
    if (std::string(kind) == "parameter") return core::SymbolKind::Parameter;
    return core::SymbolKind::Callback;
}

core::SymbolId addSymbol(Module& r, const std::string& name, const char* kind, std::string& error) {
    if (name.empty()) {
        error = std::string("empty ") + kind + " name";
        return false;
    }
    const auto id = r.symbolTable.declare(symbolKind(kind), name);
    if (id == core::InvalidSymbolId) {
        error = "duplicate symbol: " + name;
        return core::InvalidSymbolId;
    }
    return id;
}

} // namespace

std::unique_ptr<Module> lower(const ast::Module& a, std::string& error) {
    error.clear();

    // The AST's order vector is the only source of truth for declaration order.
    // Typed vectors are storage/index spaces; iterating them independently would
    // silently reorder declarations such as alias/const/enum/struct/callback.
    if (!validOrder(a, error)) {
        return {};
    }

    auto r = std::make_unique<Module>();
    r->documentation = a.documentation;
    r->nameSpace = a.nameSpace;
    for (const auto& d : a.attributeDecls) r->attributeDefinitions.push_back({d.first, static_cast<core::AttributeType>(d.second)});
    r->endian = canonicalEndian(a.endian);

    for (const auto& ref : a.order) {
        core::SymbolId id = core::InvalidSymbolId;
        switch (ref.kind) {
        case ast::TopLevelRef::Kind::Alias: id=addSymbol(*r,a.aliases[ref.index].name,"alias",error); break;
        case ast::TopLevelRef::Kind::Struct: if(!a.structs[ref.index]){error="null struct declaration";return{};} id=addSymbol(*r,a.structs[ref.index]->name,"struct",error);break;
        case ast::TopLevelRef::Kind::Enum: id=addSymbol(*r,a.enums[ref.index].name,"enum",error);break;
        case ast::TopLevelRef::Kind::Const: id=addSymbol(*r,a.constants[ref.index].name,"constant",error);break;
        case ast::TopLevelRef::Kind::Callback: id=addSymbol(*r,a.callbacks[ref.index].name,"callback",error);break;
        case ast::TopLevelRef::Kind::Parameter: id=addSymbol(*r,a.parameters[ref.index].name,"parameter",error);break;
        }
        if(id==core::InvalidSymbolId || !error.empty()) return {};
    }
    // Build the module scope from all module symbols first. Enum items get
    // stable semantic identities too; qualified names are always visible,
    // while a short item name is visible only when globally unambiguous.
    core::Scope moduleScope;
    // Qualified names are the canonical identities in SymbolTable. Unqualified
    // names of declarations owned by the current module are additionally
    // exposed in the module scope so lexical resolution can choose a local
    // declaration before falling back to the module declaration. This keeps
    // value-name binding in NameResolver instead of ParserDriver.
    for(const auto& sym:r->symbolTable.all()) moduleScope.declare(sym.name,sym.id);
    const std::string modulePrefix = a.nameSpace.empty() ? std::string{} : a.nameSpace + "::";
    auto exposeCurrentModuleName = [&](const std::string& canonical, core::SymbolId id) {
        if (modulePrefix.empty()) {
            moduleScope.declare(canonical, id);
        } else if (canonical.rfind(modulePrefix, 0) == 0) {
            const auto local = canonical.substr(modulePrefix.size());
            if (!local.empty() && local.find("::") == std::string::npos)
                moduleScope.declare(local, id);
        }
    };
    for(const auto& sym:r->symbolTable.all()) exposeCurrentModuleName(sym.name, sym.id);
    std::unordered_map<std::string, std::size_t> enumItemCounts;
    for(const auto& e:a.enums) for(const auto& item:e.items) ++enumItemCounts[item.name];
    for(const auto& e:a.enums) {
        const auto enumId = r->symbolTable.findId(e.name);
        if (enumId == core::InvalidSymbolId) { error = "internal enum symbol missing: " + e.name; return {}; }
        for (const auto& item:e.items) {
            const std::string qualified = e.name + "::" + item.name;
            const auto id = r->symbolTable.declare(core::SymbolKind::EnumItem, qualified);
            if (id == core::InvalidSymbolId) { error = "duplicate enum item symbol: " + qualified; return {}; }
            moduleScope.declare(qualified,id);
            if (enumItemCounts[item.name] == 1) moduleScope.declare(item.name,id);
        }
    }
    core::NameResolver moduleResolver(r->symbolTable, &moduleScope);
    ResolutionContext ctx{&moduleResolver, &moduleScope};

    for (const auto& ref : a.order) {
        switch (ref.kind) {
        case ast::TopLevelRef::Kind::Alias: {
            const auto& x = a.aliases[ref.index];
            const auto symbol = r->symbolTable.findId(x.name);
            Alias al; al.symbol=symbol; al.attributes=canonicalAttributes(x.attributes); al.documentation=x.documentation; al.name=x.name; al.target=lowerType(x.target, ctx, error); r->aliases.push_back(std::move(al));
            r->order.push_back({TopLevelRef::Kind::Alias, r->aliases.size() - 1});
            break;
        }
        case ast::TopLevelRef::Kind::Enum: {
            const auto& x = a.enums[ref.index];
            const auto symbol = r->symbolTable.findId(x.name);
            Enum e; e.symbol=symbol;
            e.attributes = canonicalAttributes(x.attributes);
            e.documentation = x.documentation;
            e.name = x.name;
            if (x.underlying) {
                e.hasUnderlying = true;
                e.underlying = lowerType(*x.underlying, ctx, error);
            }
            core::Scope enumScope(&moduleScope);
            for (const auto& i : x.items) {
                const auto itemId = r->symbolTable.findId(x.name + "::" + i.name);
                if (itemId == core::InvalidSymbolId || !enumScope.declare(i.name, itemId)) {
                    error = "duplicate enum item: " + x.name + "::" + i.name;
                    return {};
                }
            }
            core::NameResolver enumResolver(r->symbolTable, &enumScope);
            ResolutionContext enumCtx{&enumResolver, &enumScope};
            for (const auto& i : x.items) {
                EnumItem item;
                item.symbol = r->symbolTable.findId(x.name + "::" + i.name);
                item.name=i.name;
                item.value=lowerExpr(i.value.get(), enumCtx, error);
                item.documentation=i.documentation;
                e.items.push_back(std::move(item));
            }
            r->enums.push_back(std::move(e));
            r->order.push_back({TopLevelRef::Kind::Enum, r->enums.size() - 1});
            break;
        }
        case ast::TopLevelRef::Kind::Const: {
            const auto& x = a.constants[ref.index];
            const auto symbol = r->symbolTable.findId(x.name);
            Constant cc; cc.symbol=symbol; cc.attributes=canonicalAttributes(x.attributes); cc.documentation=x.documentation; cc.name=x.name; cc.expr=lowerExpr(x.expr.get(), ctx, error); cc.computed=x.computed; r->constants.push_back(std::move(cc));
            r->order.push_back({TopLevelRef::Kind::Const, r->constants.size() - 1});
            break;
        }
        case ast::TopLevelRef::Kind::Callback: {
            const auto& x = a.callbacks[ref.index];
            const auto symbol = r->symbolTable.findId(x.name);
            CallbackDecl c; c.symbol=symbol;
            c.attributes = canonicalAttributes(x.attributes);
            c.documentation = x.documentation;
            c.name = x.name;
            c.direction = canonicalDirection(x.direction);
            c.hasParameter = x.parameter.has_value();
            if (c.hasParameter) c.parameter = *x.parameter;
            r->callbacks.push_back(std::move(c));
            r->order.push_back({TopLevelRef::Kind::Callback, r->callbacks.size() - 1});
            break;
        }
        case ast::TopLevelRef::Kind::Parameter: {
            const auto& x = a.parameters[ref.index];
            Parameter q; q.symbol = r->symbolTable.findId(x.name); q.attributes = canonicalAttributes(x.attributes); q.documentation = x.documentation; q.name = x.name; q.type = lowerType(x.type, ctx, error);
            if (!error.empty()) return {};
            r->parameters.push_back(std::move(q));
            r->order.push_back({TopLevelRef::Kind::Parameter, r->parameters.size() - 1});
            break;
        }
        case ast::TopLevelRef::Kind::Struct: {
            const auto& x = a.structs[ref.index];
            if (!x) {
                error = "null struct declaration";
                return {};
            }
            const auto symbol = r->symbolTable.findId(x->name);

            auto s = std::make_unique<Struct>();
            s->symbol = symbol;
            s->attributes = canonicalAttributes(x->attributes);
            s->documentation = x->documentation;
            s->name = x->name;
            s->endian = canonicalEndian(x->endian);
            core::Scope structScope(&moduleScope);
            if (!prepareScope(x->members, structScope, r->symbolTable, error)) return {};
            core::NameResolver structResolver(r->symbolTable, &structScope);
            ResolutionContext structCtx{&structResolver, &structScope};
            for (const auto& req : x->requirements) {
                auto q = lowerExpr(req.get(), structCtx, error);
                if (!q) return {};
                s->requirements.push_back(std::move(q));
            }
            for (const auto& m : x->members) {
                auto q = lowerMember(m.get(), structCtx, r->symbolTable, error);
                if (!q) return {};
                s->members.push_back(std::move(q));
            }
            if (!duplicateFields(s->members, error)) return {};

            r->structs.push_back(std::move(s));
            r->order.push_back({TopLevelRef::Kind::Struct, r->structs.size() - 1});
            break;
        }
        }
    }

    return r;
}

} // namespace embx::ir
