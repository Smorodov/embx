#include "parser/ParserDriver.h"
#include "EmbXLexer.h"
#include "EmbXParser.h"
#include "EmbXBaseVisitor.h"
#include <antlr4-runtime.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
using namespace antlr4;

namespace embx::parser {
namespace {
struct Builder : EmbXBaseVisitor {
    std::unique_ptr<ast::Module> m = std::make_unique<ast::Module>();
    std::unique_ptr<ast::Member> last;
    std::string nameSpace;
    struct ImportSpec { std::string path; std::string alias; };
    std::vector<ImportSpec> imports;

    static std::string text(tree::ParseTree* t) { return t ? t->getText() : ""; }

    static std::string normalizeDoc(const std::string& raw) {
        if (raw.rfind("///", 0) == 0) {
            std::string s = raw.substr(3);
            if (!s.empty() && s.front() == ' ') s.erase(0, 1);
            return s;
        }
        if (raw.rfind("/**", 0) == 0 && raw.size() >= 5 && raw.substr(raw.size()-2) == "*/") {
            std::string body = raw.substr(3, raw.size()-5);
            std::istringstream in(body);
            std::ostringstream out;
            std::string line; bool first=true;
            std::vector<std::string> lines;
            while (std::getline(in, line)) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                std::size_t i=0; while(i<line.size() && (line[i]==' ' || line[i]=='\t')) ++i;
                if (i<line.size() && line[i]=='*') { ++i; if(i<line.size() && line[i]==' ') ++i; }
                std::string cleaned = line.substr(i);
                while (!cleaned.empty() && (cleaned.back() == ' ' || cleaned.back() == '\t' || cleaned.back() == '\r')) cleaned.pop_back();
                lines.push_back(std::move(cleaned));
            }
            while (!lines.empty() && lines.front().empty()) lines.erase(lines.begin());
            while (!lines.empty() && lines.back().empty()) lines.pop_back();
            for (const auto& x : lines) {
                if (!first) out << '\n';
                out << x; first=false;
            }
            std::string result = out.str();
            while (!result.empty() && (result.back() == ' ' || result.back() == '\t' || result.back() == '\r' || result.back() == '\n')) result.pop_back();
            return result;
        }
        return raw;
    }

    // The module documentation is the leading documentation comment of the
    // source file. Module documentation is a source-file property and is extracted from the leading
    // documentation comment independently of generated parser context details. Declaration documentation continues to
    // come exclusively from the parser contexts below.
    // Extract exactly the first leading documentation comment from the source.
    // This is deliberately independent of the ANTLR parse tree: module documentation
    // is a source-file property, while declaration documentation is attached by the
    // corresponding parser contexts.  Only the first leading DOC_COMMENT belongs to
    // the module; subsequent documentation comments are handled by item/member rules.
    static std::string leadingDoc(const std::string& source) {
        std::size_t pos = 0;
        if (source.size() >= 3 &&
            static_cast<unsigned char>(source[0]) == 0xEF &&
            static_cast<unsigned char>(source[1]) == 0xBB &&
            static_cast<unsigned char>(source[2]) == 0xBF) {
            pos = 3;
        }
        while (pos < source.size() &&
               (source[pos] == ' ' || source[pos] == '\t' ||
                source[pos] == '\r' || source[pos] == '\n')) {
            ++pos;
        }
        if (source.compare(pos, 3, "///") == 0) {
            const auto end = source.find_first_of("\r\n", pos);
            return normalizeDoc(source.substr(pos, end == std::string::npos
                                                       ? std::string::npos : end - pos));
        }
        if (source.compare(pos, 3, "/**") == 0) {
            const auto end = source.find("*/", pos + 3);
            if (end != std::string::npos) {
                return normalizeDoc(source.substr(pos, end + 2 - pos));
            }
        }
        return {};
    }

    static std::string docs(EmbXParser::DocCommentContext* c) {
        if (!c) return {};
        std::ostringstream out;
        bool first=true;
        for (auto* t : c->DOC_COMMENT()) {
            if (!first) out << "\n";
            out << normalizeDoc(t->getText());
            first=false;
        }
        return out.str();
    }

    std::string pendingDocumentation;

    static std::unique_ptr<ast::Expr> cloneExpr(const ast::Expr* src) {
        if (!src) return {};
        auto e = std::make_unique<ast::Expr>();
        e->kind = src->kind;
        e->text = src->text;
        e->op = src->op;
        e->left = cloneExpr(src->left.get());
        e->right = cloneExpr(src->right.get());
        return e;
    }
    static ast::Endian endian(const std::string& e) {
        if(e=="little") return ast::Endian::Little;
        if(e=="big") return ast::Endian::Big;
        if(e=="native") return ast::Endian::Native;
        return ast::Endian::Inherit;
    }

    std::unique_ptr<ast::Expr> expr(EmbXParser::ExprContext* c) {
        if(!c) return {};
        auto e=std::make_unique<ast::Expr>(); e->text=text(c);
        if(c->op && c->expr().size()==2) {
            e->kind=ast::ExprKind::Binary; e->op=c->op->getText();
            e->left=expr(c->expr(0)); e->right=expr(c->expr(1)); return e;
        }
        if(c->op==nullptr && c->expr().size()==1 && c->primary()==nullptr) {
            e->kind=ast::ExprKind::Unary; e->op="-"; e->right=expr(c->expr(0)); return e;
        }
        if(c->primary()) {
            auto* p=c->primary();
            if(p->expr()) { e->kind=ast::ExprKind::Parenthesized; e->left=expr(p->expr()); return e; }
            if(p->qualifiedName() || p->NEXT() || p->SIZE_IN_BYTES() || p->MIN_SIZE_IN_BYTES() || p->MAX_SIZE_IN_BYTES()) e->kind=ast::ExprKind::Identifier;
            else e->kind=ast::ExprKind::Literal;
            return e;
        }
        return e;
    }

    ast::TypeRef tr(EmbXParser::TypeRefContext* c) {
        ast::TypeRef t; if(!c) return t;
        t.name=c->baseType()?text(c->baseType()):text(c->qualifiedName());
        auto* s = c->typeSuffix();
        if(s) {
            ast::TypeSuffix ts;
            if(s->STAR()) ts.dynamic = true;
            else ts.expr = expr(s->expr());
            t.suffixes.push_back(std::move(ts));
        }
        if (c->terminatedSequenceSuffix()) {
            for (auto* h : c->terminatedSequenceSuffix()->HEX()) {
                const auto value = std::stoul(text(h), nullptr, 16);
                if (value > 0xFFu) return {};
                t.terminator.push_back(static_cast<std::uint8_t>(value));
            }
            t.maxPayload = static_cast<std::uint64_t>(std::stoull(text(c->terminatedSequenceSuffix()->INT())));
        }
        return t;
    }

    static ast::AttributeType attributeType(const std::string& t) {
        if (t == "f32" || t == "f64") return ast::AttributeType::Float;
        if (t == "string") return ast::AttributeType::String;
        return ast::AttributeType::Integer;
    }

    std::vector<ast::Attribute> attrs(EmbXParser::AttributesContext* c) {
        std::vector<ast::Attribute> out;
        if (!c) return out;
        for (auto* e : c->attributeEntry()) {
            ast::Attribute a;
            a.name = text(e->ID(0));
            if (e->literal()) { a.hasValue = true; a.value = text(e->literal()); }
            else if (e->ID().size() > 1) { a.hasValue = true; a.value = text(e->ID(1)); }
            else if (e->TRUE()) { a.hasValue = true; a.value = text(e->TRUE()); }
            else if (e->FALSE()) { a.hasValue = true; a.value = text(e->FALSE()); }
            out.push_back(std::move(a));
        }
        return out;
    }

    void fillModifiers(ast::Field& f, const std::vector<EmbXParser::FieldModifierContext*>& mods) {
        for(auto* x:mods) {
            ast::FieldModifier fm;
            if(x->endian()) { fm.kind=ast::ModifierKind::Endian; fm.endian=endian(text(x->endian())); f.endian=fm.endian; }
            else if(x->expr()) { fm.kind=ast::ModifierKind::Length; fm.expr=expr(x->expr()); }
            else if(x->INT()) { fm.kind=ast::ModifierKind::BitWidth; fm.bits=std::stoi(text(x->INT())); f.bits=fm.bits; }
            f.modifiers.push_back(std::move(fm));
        }
    }

    std::unique_ptr<ast::Field> field(EmbXParser::FieldDeclContext* c) {
        auto f=std::make_unique<ast::Field>(); f->attributes=attrs(c->attributes()); f->documentation=pendingDocumentation; pendingDocumentation.clear(); f->name=text(c->ID()); f->type=tr(c->typeRef()); fillModifiers(*f,c->fieldModifier()); if(c->transform()){ auto t=std::make_unique<ast::Transform>(); t->name=text(c->transform()->ID()); for(auto* e:c->transform()->expr()) t->arguments.push_back(expr(e)); f->transform=std::move(t); } if(c->literal()) f->assertion=text(c->literal()); return f;
    }

    void addMembers(const std::vector<EmbXParser::MemberContext*>& ms, std::vector<std::unique_ptr<ast::Member>>& out) {
        for(auto* x:ms) { x->accept(this); if(last) { out.push_back(std::move(last)); last.reset(); } }
    }

    std::any visitItem(EmbXParser::ItemContext* c) override {
        pendingDocumentation = docs(c->docComment());
        return visitChildren(c);
    }
    std::any visitMember(EmbXParser::MemberContext* c) override {
        pendingDocumentation = docs(c->docComment());
        return visitChildren(c);
    }
    std::any visitAttributeDecl(EmbXParser::AttributeDeclContext* c) override {
        const std::string name = text(c->ID());
        ast::AttributeType t = ast::AttributeType::Marker;
        if (c->attributeType()) t = attributeType(text(c->attributeType()));
        m->attributeDecls.push_back({name, t});
        return {};
    }
    std::any visitModule(EmbXParser::ModuleContext* c) override {
        // Grammar deliberately permits exactly one leading DOC_COMMENT for the
        // module. Any following documentation comment belongs to the first
        // declaration and is therefore handled by visitItem().
        if (c->DOC_COMMENT()) {
            m->documentation = normalizeDoc(c->DOC_COMMENT()->getText());
        }
        return visitChildren(c);
    }
    std::any visitNamespaceDecl(EmbXParser::NamespaceDeclContext* c) override { nameSpace=text(c->qualifiedName()); m->nameSpace=nameSpace; return {}; }
    std::any visitImportDecl(EmbXParser::ImportDeclContext* c) override { ImportSpec x; x.path=text(c->STRING()); if(x.path.size()>=2) x.path=x.path.substr(1,x.path.size()-2); if(c->ID()) x.alias=text(c->ID()); imports.push_back(std::move(x)); return {}; }
    std::any visitEndianDirective(EmbXParser::EndianDirectiveContext* c) override { m->endian=endian(text(c->endian())); return {}; }
    std::any visitConstDecl(EmbXParser::ConstDeclContext* c) override { ast::Const x; x.attributes=attrs(c->attributes()); x.documentation=pendingDocumentation; pendingDocumentation.clear(); x.name=text(c->ID()); x.expr=expr(c->expr()); m->order.push_back({ast::TopLevelRef::Kind::Const,m->constants.size()}); m->constants.push_back(std::move(x)); return {}; }
    std::any visitComputedDecl(EmbXParser::ComputedDeclContext* c) override { ast::Const x; x.attributes=attrs(c->attributes()); x.documentation=pendingDocumentation; pendingDocumentation.clear(); x.name=text(c->ID()); x.expr=expr(c->expr()); x.computed=true; m->order.push_back({ast::TopLevelRef::Kind::Const,m->constants.size()}); m->constants.push_back(std::move(x)); return {}; }

    std::any visitEnumDecl(EmbXParser::EnumDeclContext* c) override {
        ast::Enum e; e.attributes=attrs(c->attributes()); e.documentation=pendingDocumentation; pendingDocumentation.clear(); e.name=text(c->ID()); if(c->typeRef()) e.underlying=std::make_unique<ast::TypeRef>(tr(c->typeRef()));
        for(auto* i:c->enumItem()) { ast::EnumItem x; x.documentation=docs(i->docComment()); x.name=text(i->ID()); x.value=expr(i->expr()); e.items.push_back(std::move(x)); }
        m->order.push_back({ast::TopLevelRef::Kind::Enum,m->enums.size()}); m->enums.push_back(std::move(e)); return {};
    }
    std::any visitStructDecl(EmbXParser::StructDeclContext* c) override {
        auto s=std::make_unique<ast::Struct>(); s->attributes=attrs(c->attributes()); s->documentation=pendingDocumentation; pendingDocumentation.clear(); s->name=text(c->ID()); if(c->endian()) s->endian=endian(text(c->endian()));
        for (auto* r : c->requiresDecl()) s->requirements.push_back(expr(r->expr()));
        addMembers(c->member(),s->members); m->order.push_back({ast::TopLevelRef::Kind::Struct,m->structs.size()}); m->structs.push_back(std::move(s));
        return {};
    }
    std::any visitVirtualDecl(EmbXParser::VirtualDeclContext* c) override { auto v=std::make_unique<ast::Virtual>(); v->attributes=attrs(c->attributes()); v->documentation=pendingDocumentation; pendingDocumentation.clear(); v->name=text(c->ID()); v->expression=expr(c->expr()); last=std::move(v); return {}; }
    std::any visitAliasDecl(EmbXParser::AliasDeclContext* c) override { auto a=std::make_unique<ast::Alias>(); a->attributes=attrs(c->attributes()); a->documentation=pendingDocumentation; pendingDocumentation.clear(); a->name=text(c->ID(0)); a->target=text(c->ID(1)); last=std::move(a); return {}; }
    std::any visitFieldDecl(EmbXParser::FieldDeclContext* c) override { last=field(c); return {}; }
    std::any visitBitsBlock(EmbXParser::BitsBlockContext* c) override {
        auto b=std::make_unique<ast::Bits>(); b->attributes=attrs(c->attributes()); b->documentation=pendingDocumentation; pendingDocumentation.clear();
        for(auto* x:c->bitField()) { auto f=std::make_unique<ast::Field>(); f->documentation=docs(x->docComment()); f->name=text(x->ID()); f->type=tr(x->typeRef()); if(x->INT()) f->bits=std::stoi(text(x->INT())); fillModifiers(*f,x->fieldModifier()); if(x->literal()) f->assertion=text(x->literal()); b->fields.push_back(std::move(f)); }
        last=std::move(b); return {};
    }
    std::unique_ptr<ast::VariantCase> variantCase(EmbXParser::VariantCaseContext* x) {
        auto v=std::make_unique<ast::VariantCase>(); v->tag=expr(x->expr()); auto* b=x->variantBody(); if(b->typeRef()) v->type=std::make_unique<ast::TypeRef>(tr(b->typeRef())); else addMembers(b->member(),v->members); return v;
    }
    std::any visitConditionalDecl(EmbXParser::ConditionalDeclContext* c) override {
        auto x = std::make_unique<ast::Conditional>();
        x->attributes = attrs(c->attributes());
        x->documentation = pendingDocumentation; pendingDocumentation.clear();
        x->condition = expr(c->expr());
        bool inElse = false;
        for (auto* child : c->children) {
            if (child == c->ELSE()) { inElse = true; continue; }
            if (auto* mc = dynamic_cast<EmbXParser::MemberContext*>(child)) {
                mc->accept(this);
                if (last) {
                    if (inElse) x->elseMembers.push_back(std::move(last));
                    else x->thenMembers.push_back(std::move(last));
                    last.reset();
                }
            }
        }
        last = std::move(x); return {};
    }
    std::any visitVariantDecl(EmbXParser::VariantDeclContext* c) override {
        auto v=std::make_unique<ast::Variant>(); v->attributes=attrs(c->attributes()); v->documentation=pendingDocumentation; pendingDocumentation.clear(); v->name=c->ID()?text(c->ID()):""; v->discriminator=expr(c->expr());
        for(auto* x:c->variantCase()) { auto vc=variantCase(x); v->cases.push_back(std::move(*vc)); }
        if(c->variantDefault()) { auto* b=c->variantDefault()->variantBody(); if(b->typeRef()) v->defaultType=std::make_unique<ast::TypeRef>(tr(b->typeRef())); else addMembers(b->member(),v->defaultMembers); }
        last=std::move(v); return {};
    }
    std::any visitBlockDecl(EmbXParser::BlockDeclContext* c) override { auto b=std::make_unique<ast::Block>(); b->attributes=attrs(c->attributes()); b->documentation=pendingDocumentation; pendingDocumentation.clear(); b->name=text(c->ID()); b->size=expr(c->expr()); addMembers(c->member(),b->members); last=std::move(b); return {}; }
    std::any visitAtDecl(EmbXParser::AtDeclContext* c) override { auto a=std::make_unique<ast::At>(); a->attributes=attrs(c->attributes()); a->documentation=pendingDocumentation; pendingDocumentation.clear(); a->offset=expr(c->expr()); addMembers(c->member(),a->members); last=std::move(a); return {}; }
    std::any visitAlignDecl(EmbXParser::AlignDeclContext* c) override { auto a=std::make_unique<ast::Align>(expr(c->expr())); a->attributes=attrs(c->attributes()); a->documentation=pendingDocumentation; pendingDocumentation.clear(); last=std::move(a); return {}; }
    std::any visitCallbackUse(EmbXParser::CallbackUseContext* c) override { auto x=std::make_unique<ast::Callback>(); x->attributes=attrs(c->attributes()); x->documentation=pendingDocumentation; pendingDocumentation.clear(); x->name=text(c->ID()); x->direction=ast::callbackDirectionFromName(x->name); if(c->argList()) for(auto* e:c->argList()->expr()) x->args.push_back(expr(e)); last=std::move(x); return {}; }
    std::any visitCallbackDecl(EmbXParser::CallbackDeclContext* c) override { ast::CallbackDecl x; x.attributes=attrs(c->attributes()); x.documentation=pendingDocumentation; pendingDocumentation.clear(); x.name=c->ID()[0]->getText(); x.direction=ast::callbackDirectionFromName(x.name); if(c->ID().size()>1) x.parameter=c->ID()[1]->getText(); m->order.push_back({ast::TopLevelRef::Kind::Callback,m->callbacks.size()}); m->callbacks.push_back(std::move(x)); return {}; }
    std::any visitParameterDecl(EmbXParser::ParameterDeclContext* c) override { ast::Parameter x; x.attributes=attrs(c->attributes()); x.documentation=pendingDocumentation; pendingDocumentation.clear(); x.name=text(c->ID()); x.type=tr(c->typeRef()); m->order.push_back({ast::TopLevelRef::Kind::Parameter,m->parameters.size()}); m->parameters.push_back(std::move(x)); return {}; }
    std::any visitTypeAlias(EmbXParser::TypeAliasContext* c) override { ast::TypeAlias a; a.attributes=attrs(c->attributes()); a.documentation=pendingDocumentation; pendingDocumentation.clear(); a.name=text(c->ID()); a.target=tr(c->typeRef()); m->order.push_back({ast::TopLevelRef::Kind::Alias,m->aliases.size()}); m->aliases.push_back(std::move(a)); return {}; }
};
}
namespace {

std::string unquote(const std::string& s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') return s.substr(1, s.size()-2);
    return s;
}

std::string joinName(const std::string& ns, const std::string& name) {
    if (ns.empty()) return name;
    if (name.empty()) return ns;
    return ns + "::" + name;
}

bool hasScope(const std::string& s) { return s.find("::") != std::string::npos; }

void rewriteExpr(ast::Expr* e,
                 const std::unordered_map<std::string,std::string>& aliases) {
    if (!e) return;
    if (e->kind == ast::ExprKind::Identifier) {
        auto pos=e->text.find("::");
        if (pos != std::string::npos) {
            auto it=aliases.find(e->text.substr(0,pos));
            if(it!=aliases.end()) e->text=it->second+e->text.substr(pos);
        }
    }
    rewriteExpr(e->left.get(),aliases);
    rewriteExpr(e->right.get(),aliases);
}

void rewriteType(ast::TypeRef& t, const std::string& ns,
                 const std::unordered_set<std::string>& types,
                 const std::unordered_map<std::string,std::string>& aliases) {
    if (!t.name.empty() && !hasScope(t.name) && types.count(t.name)) t.name=joinName(ns,t.name);
    else if (hasScope(t.name)) {
        auto pos=t.name.find("::");
        auto it=aliases.find(t.name.substr(0,pos));
        if(it!=aliases.end()) t.name=it->second+t.name.substr(pos);
    }
    for(auto& x:t.suffixes) rewriteExpr(x.expr.get(),aliases);
}

void rewriteMembers(std::vector<std::unique_ptr<ast::Member>>& ms, const std::string& ns,
                    const std::unordered_set<std::string>& types,
                    const std::unordered_set<std::string>& callbacks,
                    const std::unordered_map<std::string,std::string>& aliases) {
    for(auto& m:ms) {
        if(auto* f=dynamic_cast<ast::Field*>(m.get())) { rewriteType(f->type,ns,types,aliases); for(auto& x:f->modifiers) rewriteExpr(x.expr.get(),aliases); if(f->transform) for(auto& e:f->transform->arguments) rewriteExpr(e.get(),aliases); }
        else if(auto* v=dynamic_cast<ast::Virtual*>(m.get())) rewriteExpr(v->expression.get(),aliases);
        else if(auto* a=dynamic_cast<ast::Alias*>(m.get())) { (void)a; }
        else if(auto* b=dynamic_cast<ast::Bits*>(m.get())) for(auto& f:b->fields) { rewriteType(f->type,ns,types,aliases); for(auto& x:f->modifiers) rewriteExpr(x.expr.get(),aliases); if(f->transform) for(auto& e:f->transform->arguments) rewriteExpr(e.get(),aliases); }
        else if(auto* v=dynamic_cast<ast::Variant*>(m.get())) { rewriteExpr(v->discriminator.get(),aliases); for(auto& c:v->cases){rewriteExpr(c.tag.get(),aliases); if(c.type) rewriteType(*c.type,ns,types,aliases); else rewriteMembers(c.members,ns,types,callbacks,aliases);} if(v->defaultType) rewriteType(*v->defaultType,ns,types,aliases); else rewriteMembers(v->defaultMembers,ns,types,callbacks,aliases); }
        else if(auto* b=dynamic_cast<ast::Block*>(m.get())) { rewriteExpr(b->size.get(),aliases); rewriteMembers(b->members,ns,types,callbacks,aliases); }
        else if(auto* a=dynamic_cast<ast::At*>(m.get())) { rewriteExpr(a->offset.get(),aliases); rewriteMembers(a->members,ns,types,callbacks,aliases); }
        else if(auto* a=dynamic_cast<ast::Align*>(m.get())) rewriteExpr(a->alignment.get(),aliases);
        else if(auto* c=dynamic_cast<ast::Callback*>(m.get())) { if(!c->name.empty() && c->name.find("::") == std::string::npos && callbacks.count(c->name)) c->name=joinName(ns,c->name); for(auto& e:c->args) rewriteExpr(e.get(),aliases); }
    }
}

void qualifyModule(ast::Module& m, const std::unordered_map<std::string,std::string>& aliases) {
    const std::string ns=m.nameSpace;
    std::unordered_set<std::string> types, callbacks;
    for(auto& a:m.aliases){types.insert(a.name);}
    for(auto& s:m.structs) types.insert(s->name);
    for(auto& e:m.enums) types.insert(e.name);
    for(auto& c:m.callbacks) callbacks.insert(c.name);
    for(auto& a:m.aliases){a.name=joinName(ns,a.name); rewriteType(a.target,ns,types,aliases);}
    for(auto& s:m.structs){s->name=joinName(ns,s->name); rewriteMembers(s->members,ns,types,callbacks,aliases);}
    for(auto& e:m.enums){e.name=joinName(ns,e.name); if(e.underlying) rewriteType(*e.underlying,ns,types,aliases); for(auto& i:e.items) rewriteExpr(i.value.get(),aliases);}
    for(auto& c:m.constants){c.name=joinName(ns,c.name); rewriteExpr(c.expr.get(),aliases);}
    for(auto& c:m.callbacks)c.name=joinName(ns,c.name);
    for(auto& p:m.parameters){p.name=joinName(ns,p.name); rewriteType(p.type,ns,types,aliases);}
    // Callback uses are resolved in the same pass as the containing members.
}

void mergeModule(ast::Module& dst, ast::Module&& src) {
    if(dst.endian==ast::Endian::Inherit) dst.endian=src.endian;
    if (dst.documentation.empty()) dst.documentation = src.documentation;
    for (const auto& d : src.attributeDecls) dst.attributeDecls.push_back(d);

    // Preserve the source module's declaration order exactly. The typed
    // vectors are storage/index spaces; src.order is the only authoritative
    // ordering information. This matters even when there are no imports:
    // loadRecursive() normalizes every parsed module through this merge.
    for(const auto& ref : src.order) {
        switch(ref.kind) {
        case ast::TopLevelRef::Kind::Alias: {
            const auto index=dst.aliases.size();
            dst.aliases.push_back(std::move(src.aliases[ref.index]));
            dst.order.push_back({ast::TopLevelRef::Kind::Alias,index});
            break;
        }
        case ast::TopLevelRef::Kind::Struct: {
            const auto index=dst.structs.size();
            dst.structs.push_back(std::move(src.structs[ref.index]));
            dst.order.push_back({ast::TopLevelRef::Kind::Struct,index});
            break;
        }
        case ast::TopLevelRef::Kind::Enum: {
            const auto index=dst.enums.size();
            dst.enums.push_back(std::move(src.enums[ref.index]));
            dst.order.push_back({ast::TopLevelRef::Kind::Enum,index});
            break;
        }
        case ast::TopLevelRef::Kind::Const: {
            const auto index=dst.constants.size();
            dst.constants.push_back(std::move(src.constants[ref.index]));
            dst.order.push_back({ast::TopLevelRef::Kind::Const,index});
            break;
        }
        case ast::TopLevelRef::Kind::Callback: {
            const auto index=dst.callbacks.size();
            dst.callbacks.push_back(std::move(src.callbacks[ref.index]));
            dst.order.push_back({ast::TopLevelRef::Kind::Callback,index});
            break;
        }
        case ast::TopLevelRef::Kind::Parameter: {
            const auto index=dst.parameters.size();
            dst.parameters.push_back(std::move(src.parameters[ref.index]));
            dst.order.push_back({ast::TopLevelRef::Kind::Parameter,index});
            break;
        }
        }
    }
}

std::unique_ptr<ast::Module> loadRecursive(const std::filesystem::path& path, std::string& error,
                                           std::vector<std::filesystem::path>& stack,
                                           std::unordered_set<std::string>& loaded) {
    std::filesystem::path canon;
    try { canon=std::filesystem::weakly_canonical(path); } catch(...) { canon=path; }
    const std::string key=canon.lexically_normal().string();
    for(const auto& x:stack) if(x.lexically_normal().string()==key){error="import cycle: "+key;return{};}
    if(loaded.count(key)) return std::make_unique<ast::Module>();
    std::ifstream in(path,std::ios::binary); if(!in){error="cannot open "+path.string();return{};}
    std::stringstream ss; ss<<in.rdbuf(); const std::string source=ss.str(); ANTLRInputStream input(source); EmbXLexer lexer(&input); CommonTokenStream tokens(&lexer); EmbXParser parser(&tokens); parser.removeErrorListeners();
    class EL:public BaseErrorListener{public:std::string s;void syntaxError(Recognizer*,Token*,size_t l,size_t c,const std::string& msg,std::exception_ptr)override{std::ostringstream o;o<<l<<":"<<c<<": "<<msg<<"\n";s+=o.str();}}; EL el; parser.addErrorListener(&el); auto* tree=parser.module(); if(!el.s.empty()){error=el.s;return{};}
    Builder b;
    tree->accept(&b);
    auto result=std::move(b.m); result->nameSpace=b.nameSpace;
    // The source-level rule is authoritative for module documentation.
    // Re-read it after parsing so parser traversal cannot accidentally lose it.
    result->documentation = Builder::leadingDoc(source);
    stack.push_back(canon);

    // Load imports first so aliases can be resolved before local names are canonicalized.
    std::vector<std::pair<Builder::ImportSpec,std::unique_ptr<ast::Module>>> children;
    std::unordered_map<std::string,std::string> aliases;
    std::unordered_set<std::string> importNames;
    for(const auto& imp:b.imports){
        if(!importNames.insert(imp.alias.empty()?imp.path:(imp.path+" as "+imp.alias)).second){
            error="duplicate import: "+imp.path; stack.pop_back(); return{};
        }
        auto childPath=path.parent_path()/unquote(imp.path); std::string e;
        auto child=loadRecursive(childPath,e,stack,loaded);
        if(!child){error="import "+imp.path+": "+e; stack.pop_back(); return{};}
        const std::string targetNs=child->nameSpace;
        const std::string exposed=imp.alias.empty()?targetNs:imp.alias;
        if(imp.alias.empty() && targetNs.empty()) {
            // Global modules are importable without a prefix; their declarations
            // simply join the global namespace.
        } else if(exposed.empty()) {
            error="import alias cannot be empty: "+imp.alias; stack.pop_back(); return{};
        } else if(aliases.count(exposed)) {
            error="duplicate import namespace: "+exposed; stack.pop_back(); return{};
        } else {
            aliases.emplace(exposed,targetNs);
        }
        children.emplace_back(imp.alias.empty()?Builder::ImportSpec{imp.path, exposed}:imp, std::move(child));
    }

    // Qualify this module's declarations and references. A module with no
    qualifyModule(*result,aliases);
    ast::Module merged;
    merged.nameSpace=result->nameSpace;
    merged.endian=result->endian;
    mergeModule(merged,std::move(*result));
    loaded.insert(key);

    for(auto& item:children) {
        // Imported modules are already canonicalized in their own namespace.
        // If an import alias was used, it has been resolved in the importing
        // module's references above; no renaming of the imported declarations is needed.
        mergeModule(merged,std::move(*item.second));
    }

    // The module documentation belongs to this source file, not to an imported
    // module. Re-apply it after the complete merge so import processing can
    // never replace, clear, or otherwise affect the root module documentation.
    // This is intentionally independent of ANTLR visitor/accessor behaviour.
    const std::string rootDocumentation = Builder::leadingDoc(source);
    if (!rootDocumentation.empty()) merged.documentation = rootDocumentation;

    stack.pop_back();
    return std::make_unique<ast::Module>(std::move(merged));
}
} // namespace

std::unique_ptr<ast::Module> parseFile(const std::string& path,std::string& error){
    error.clear(); std::vector<std::filesystem::path> stack; std::unordered_set<std::string> loaded;
    return loadRecursive(std::filesystem::path(path),error,stack,loaded);
}

}
