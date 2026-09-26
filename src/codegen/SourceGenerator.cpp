#include "codegen/SourceGenerator.h"
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

namespace embx::codegen {
namespace {

struct Writer {
    std::ostringstream out;
    int depth = 0;
    void line(const std::string& s = {}) { out << std::string(static_cast<std::size_t>(depth) * 4u, ' ') << s << '\n'; }
    void push() { ++depth; }
    void pop() { --depth; }
};

int precedence(const ast::Expr* e) {
    if (!e) return 100;
    if (e->kind == ast::ExprKind::Parenthesized || e->kind == ast::ExprKind::Literal || e->kind == ast::ExprKind::Identifier) return 100;
    if (e->kind == ast::ExprKind::Unary) return 90;
    if (e->kind != ast::ExprKind::Binary) return -1;
    if (e->op == "*" || e->op == "/" || e->op == "%") return 70;
    if (e->op == "+" || e->op == "-") return 60;
    if (e->op == "==" || e->op == "!=" || e->op == "<" || e->op == "<=" || e->op == ">" || e->op == ">=") return 50;
    if (e->op == "&&") return 40;
    if (e->op == "||") return 30;
    return -1;
}

std::string expr(const ast::Expr* e, int parentPrec = -1, bool rightChild = false) {
    if (!e) return {};
    if (e->kind == ast::ExprKind::Literal || e->kind == ast::ExprKind::Identifier) return e->text;
    if (e->kind == ast::ExprKind::Parenthesized) return "(" + expr(e->left.get()) + ")";
    if (e->kind == ast::ExprKind::Unary) {
        std::string s = e->op + expr(e->right.get(), precedence(e));
        if (precedence(e) < parentPrec) s = "(" + s + ")";
        return s;
    }
    if (e->kind == ast::ExprKind::Binary) {
        const int p = precedence(e);
        std::string l = expr(e->left.get(), p, false);
        std::string r = expr(e->right.get(), p, true);
        // A same-precedence right child is parenthesized to preserve the AST tree.
        if (e->right && e->right->kind == ast::ExprKind::Binary && precedence(e->right.get()) == p) r = "(" + r + ")";
        std::string s = l + " " + e->op + " " + r;
        if (p < parentPrec || (rightChild && p == parentPrec)) s = "(" + s + ")";
        return s;
    }
    return {};
}


bool validateExpr(const ast::Expr* e, std::string& error, const char* context) {
    if (!e) { error = std::string("source generator: missing expression in ") + context; return false; }
    switch (e->kind) {
    case ast::ExprKind::Literal:
    case ast::ExprKind::Identifier:
        if (e->text.empty()) { error = std::string("source generator: empty expression in ") + context; return false; }
        return true;
    case ast::ExprKind::Parenthesized:
        return validateExpr(e->left.get(), error, context);
    case ast::ExprKind::Unary:
        if (e->op.empty()) { error = std::string("source generator: empty unary operator in ") + context; return false; }
        return validateExpr(e->right.get(), error, context);
    case ast::ExprKind::Binary:
        if (e->op.empty()) { error = std::string("source generator: empty binary operator in ") + context; return false; }
        return validateExpr(e->left.get(), error, context) && validateExpr(e->right.get(), error, context);
    }
    error = std::string("source generator: unsupported expression kind in ") + context;
    return false;
}

bool validateTypeRef(const ast::TypeRef& t, std::string& error, const char* context) {
    if (t.name.empty()) { error = std::string("source generator: empty type name in ") + context; return false; }
    for (const auto& suffix : t.suffixes) {
        if (suffix.dynamic) {
            if (suffix.expr) { error = std::string("source generator: dynamic dimension has an expression in ") + context; return false; }
        } else if (!validateExpr(suffix.expr.get(), error, context)) return false;
    }
    if (!t.terminator.empty() && !t.maxPayload) {
        error = std::string("source generator: terminated sequence is missing max payload in ") + context;
        return false;
    }
    return true;
}

std::string localName(const std::string& name) {
    const auto pos = name.rfind("::");
    return pos == std::string::npos ? name : name.substr(pos + 2);
}

std::string endian(ast::Endian e) {
    switch (e) {
    case ast::Endian::Little: return "little";
    case ast::Endian::Big: return "big";
    case ast::Endian::Native: return "native";
    case ast::Endian::Inherit: return {};
    }
    return {};
}

std::string attributeType(ast::AttributeType t) {
    switch (t) {
    case ast::AttributeType::Marker: return {};
    case ast::AttributeType::Integer: return "u64";
    case ast::AttributeType::Float: return "f64";
    case ast::AttributeType::String: return "string";
    }
    return {};
}

void docs(Writer& w, const std::string& documentation) {
    if (documentation.empty()) return;
    std::istringstream in(documentation);
    std::string line;
    while (std::getline(in, line)) w.line("///" + (line.empty() ? std::string{} : " " + line));
}

void attrs(Writer& w, const std::vector<ast::Attribute>& as) {
    if (as.empty()) return;
    std::ostringstream s;
    s << "[";
    for (std::size_t i = 0; i < as.size(); ++i) {
        if (i) s << ", ";
        s << as[i].name;
        if (as[i].hasValue) s << " = " << as[i].value;
    }
    s << "]";
    w.line(s.str());
}

std::string typeRef(const ast::TypeRef& t) {
    std::ostringstream s;
    s << t.name;
    for (const auto& x : t.suffixes) {
        s << '[';
        if (x.dynamic) s << '*'; else s << expr(x.expr.get());
        s << ']';
    }
    if (!t.terminator.empty()) {
        s << " until";
        for (std::uint8_t b : t.terminator) {
            s << ' ' << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b)
              << std::dec << std::setfill(' ');
        }
        s << " max " << t.maxPayload.value_or(0);
    }
    return s.str();
}

void modifierText(std::ostringstream& s, const ast::FieldModifier& m) {
    switch (m.kind) {
    case ast::ModifierKind::Endian: if (auto x = endian(m.endian); !x.empty()) s << ' ' << x; break;
    case ast::ModifierKind::Length: s << " [" << expr(m.expr.get()) << ']'; break;
    case ast::ModifierKind::BitWidth: s << " (" << m.bits << ')'; break;
    }
}

bool field(Writer& w, const ast::Field& f, bool bitField, std::string& error) {
    if (!bitField && f.bits != 0) { error = "source generator cannot represent Field::bits outside a bits block"; return false; }
    docs(w, f.documentation); attrs(w, f.attributes);
    std::ostringstream s;
    s << f.name << ": " << typeRef(f.type);
    if (bitField) s << " (" << f.bits << ')';
    for (const auto& m : f.modifiers) modifierText(s, m);
    if (f.transform) {
        s << " transform " << f.transform->name << '(';
        for (std::size_t i = 0; i < f.transform->arguments.size(); ++i) {
            if (i) s << ", ";
            s << expr(f.transform->arguments[i].get());
        }
        s << ')';
    }
    if (!f.assertion.empty()) s << " = " << f.assertion;
    s << ';';
    w.line(s.str());
    return true;
}

void member(Writer&, const ast::Member*, std::string&);

void members(Writer& w, const std::vector<std::unique_ptr<ast::Member>>& ms, std::string& error) {
    for (const auto& m : ms) {
        if (!m) { error = "source generator encountered null member"; return; }
        member(w, m.get(), error);
        if (!error.empty()) return;
    }
}

void member(Writer& w, const ast::Member* m, std::string& error) {
    if (auto* f = dynamic_cast<const ast::Field*>(m)) { field(w, *f, false, error); return; }
    if (auto* v = dynamic_cast<const ast::Virtual*>(m)) {
        docs(w, v->documentation); attrs(w, v->attributes); w.line("let " + v->name + " = " + expr(v->expression.get()) + ";"); return;
    }
    if (auto* a = dynamic_cast<const ast::Alias*>(m)) {
        docs(w, a->documentation); attrs(w, a->attributes); w.line("alias " + a->name + " = " + a->target + ";"); return;
    }
    if (auto* b = dynamic_cast<const ast::Bits*>(m)) {
        docs(w, b->documentation); attrs(w, b->attributes); w.line("bits {"); w.push();
        for (const auto& f : b->fields) { if (!f) { error = "source generator encountered null bit field"; return; } if (!field(w, *f, true, error)) return; }
        w.pop(); w.line("}"); return;
    }
    if (auto* v = dynamic_cast<const ast::Variant*>(m)) {
        docs(w, v->documentation); attrs(w, v->attributes);
        std::string head = "variant" + (v->name.empty() ? std::string{} : " " + v->name) + " by " + expr(v->discriminator.get()) + " {";
        w.line(head); w.push();
        for (const auto& c : v->cases) {
            std::ostringstream s; s << expr(c.tag.get()) << ":";
            if (c.type) { s << ' ' << typeRef(*c.type) << ';'; w.line(s.str()); }
            else { w.line(s.str() + " {"); w.push(); members(w, c.members, error); if (!error.empty()) return; w.pop(); w.line("}"); }
        }
        if (v->defaultType) w.line("default: " + typeRef(*v->defaultType) + ";");
        else if (!v->defaultMembers.empty()) { w.line("default: {"); w.push(); members(w, v->defaultMembers, error); if (!error.empty()) return; w.pop(); w.line("}"); }
        w.pop(); w.line("}"); return;
    }
    if (auto* c = dynamic_cast<const ast::Conditional*>(m)) {
        docs(w, c->documentation); attrs(w, c->attributes); w.line("if (" + expr(c->condition.get()) + ") {"); w.push(); members(w, c->thenMembers, error); if (!error.empty()) return; w.pop();
        if (!c->elseMembers.empty()) { w.line("} else {"); w.push(); members(w, c->elseMembers, error); if (!error.empty()) return; w.pop(); }
        w.line("}"); return;
    }
    if (auto* b = dynamic_cast<const ast::Block*>(m)) {
        docs(w, b->documentation); attrs(w, b->attributes); std::string n = b->name.empty() ? "" : " " + b->name; w.line("block" + n + " [" + expr(b->size.get()) + "] {"); w.push(); members(w, b->members, error); if (!error.empty()) return; w.pop(); w.line("}"); return;
    }
    if (auto* a = dynamic_cast<const ast::At*>(m)) {
        docs(w, a->documentation); attrs(w, a->attributes); w.line("at(" + expr(a->offset.get()) + ") {"); w.push(); members(w, a->members, error); if (!error.empty()) return; w.pop(); w.line("}"); return;
    }
    if (auto* a = dynamic_cast<const ast::Align*>(m)) {
        docs(w, a->documentation); attrs(w, a->attributes); w.line("align(" + expr(a->alignment.get()) + ");"); return;
    }
    if (auto* c = dynamic_cast<const ast::Callback*>(m)) {
        docs(w, c->documentation); attrs(w, c->attributes); std::ostringstream s; s << "callback " << localName(c->name) << '(';
        for (std::size_t i = 0; i < c->args.size(); ++i) { if (i) s << ", "; s << expr(c->args[i].get()); } s << ");"; w.line(s.str()); return;
    }
    error = "source generator encountered unsupported AST member";
}

void topDocAttrs(Writer& w, const std::string& doc, const std::vector<ast::Attribute>& as) { docs(w, doc); attrs(w, as); }

bool generateTop(const ast::Module& m, const ast::TopLevelRef& r, Writer& w, std::string& error) {
    switch (r.kind) {
    case ast::TopLevelRef::Kind::Alias: {
        if (r.index >= m.aliases.size()) { error = "invalid AST alias order index"; return false; }
        const auto& x=m.aliases[r.index]; topDocAttrs(w,x.documentation,x.attributes); w.line("type " + localName(x.name) + " = " + typeRef(x.target) + ";"); return true;
    }
    case ast::TopLevelRef::Kind::Struct: {
        if (r.index >= m.structs.size() || !m.structs[r.index]) { error = "invalid AST struct order index"; return false; }
        const auto& x=*m.structs[r.index]; topDocAttrs(w,x.documentation,x.attributes); std::string e=endian(x.endian); w.line("struct " + localName(x.name) + (e.empty()?"":" "+e) + " {"); w.push();
        for (const auto& req:x.requirements) w.line("requires " + expr(req.get()) + ";");
        members(w,x.members,error); if(!error.empty()) return false; w.pop(); w.line("}"); return true;
    }
    case ast::TopLevelRef::Kind::Enum: {
        if (r.index >= m.enums.size()) { error="invalid AST enum order index"; return false; }
        const auto& x=m.enums[r.index]; topDocAttrs(w,x.documentation,x.attributes); std::string h="enum "+localName(x.name); if(x.underlying) h += ": "+typeRef(*x.underlying); h += " {"; w.line(h); w.push();
        for (std::size_t i=0;i<x.items.size();++i) { docs(w,x.items[i].documentation); w.line(x.items[i].name+" = "+expr(x.items[i].value.get())+(i+1<x.items.size()?",":"")); }
        w.pop(); w.line("}"); return true;
    }
    case ast::TopLevelRef::Kind::Const: {
        if (r.index >= m.constants.size()) { error="invalid AST constant order index"; return false; }
        const auto& x=m.constants[r.index]; topDocAttrs(w,x.documentation,x.attributes); w.line(std::string(x.computed?"computed ":"const ")+localName(x.name)+" = "+expr(x.expr.get())+";"); return true;
    }
    case ast::TopLevelRef::Kind::Callback: {
        if (r.index >= m.callbacks.size()) { error="invalid AST callback order index"; return false; }
        const auto& x=m.callbacks[r.index]; topDocAttrs(w,x.documentation,x.attributes); w.line("callback "+localName(x.name)+(x.parameter?"("+*x.parameter+")":"")+";"); return true;
    }
    case ast::TopLevelRef::Kind::Parameter: {
        if (r.index >= m.parameters.size()) { error="invalid AST parameter order index"; return false; }
        const auto& x=m.parameters[r.index]; topDocAttrs(w,x.documentation,x.attributes); w.line("param "+localName(x.name)+": "+typeRef(x.type)+";"); return true;
    }
    }
    error="unsupported top-level AST declaration"; return false;
}


bool validateMember(const ast::Member* m, std::string& error) {
    if (!m) { error = "source generator: null AST member"; return false; }
    if (auto* f = dynamic_cast<const ast::Field*>(m)) {
        if (!validateTypeRef(f->type, error, "field")) return false;
        for (const auto& mod : f->modifiers)
            if (mod.kind == ast::ModifierKind::Length && !validateExpr(mod.expr.get(), error, "field length")) return false;
        if (f->transform) {
            if (f->transform->name.empty()) { error = "source generator: empty transform name"; return false; }
            for (const auto& a : f->transform->arguments)
                if (!validateExpr(a.get(), error, "transform argument")) return false;
        }
        return true;
    }
    if (auto* v = dynamic_cast<const ast::Virtual*>(m)) return validateExpr(v->expression.get(), error, "virtual field");
    if (dynamic_cast<const ast::Alias*>(m)) return true;
    if (auto* b = dynamic_cast<const ast::Bits*>(m)) {
        for (const auto& f : b->fields) if (!f || !validateTypeRef(f->type, error, "bit field")) return false;
        return true;
    }
    if (auto* v = dynamic_cast<const ast::Variant*>(m)) {
        if (!validateExpr(v->discriminator.get(), error, "variant discriminator")) return false;
        for (const auto& c : v->cases) {
            if (!validateExpr(c.tag.get(), error, "variant case tag")) return false;
            if (c.type && !validateTypeRef(*c.type, error, "variant case type")) return false;
            for (const auto& x : c.members) if (!validateMember(x.get(), error)) return false;
        }
        if (v->defaultType && !validateTypeRef(*v->defaultType, error, "variant default type")) return false;
        for (const auto& x : v->defaultMembers) if (!validateMember(x.get(), error)) return false;
        return true;
    }
    if (auto* c = dynamic_cast<const ast::Conditional*>(m)) {
        if (!validateExpr(c->condition.get(), error, "conditional")) return false;
        for (const auto& x : c->thenMembers) if (!validateMember(x.get(), error)) return false;
        for (const auto& x : c->elseMembers) if (!validateMember(x.get(), error)) return false;
        return true;
    }
    if (auto* b = dynamic_cast<const ast::Block*>(m)) {
        if (!validateExpr(b->size.get(), error, "block size")) return false;
        for (const auto& x : b->members) if (!validateMember(x.get(), error)) return false;
        return true;
    }
    if (auto* a = dynamic_cast<const ast::At*>(m)) {
        if (!validateExpr(a->offset.get(), error, "at offset")) return false;
        for (const auto& x : a->members) if (!validateMember(x.get(), error)) return false;
        return true;
    }
    if (auto* a = dynamic_cast<const ast::Align*>(m)) return validateExpr(a->alignment.get(), error, "align");
    if (auto* c = dynamic_cast<const ast::Callback*>(m)) {
        if (c->name.empty()) { error = "source generator: empty callback name"; return false; }
        for (const auto& x : c->args) if (!validateExpr(x.get(), error, "callback argument")) return false;
        return true;
    }
    error = "source generator: unsupported AST member kind";
    return false;
}

bool validateModule(const ast::Module& m, std::string& error) {
    for (const auto& a : m.aliases) if (!validateTypeRef(a.target, error, "type alias")) return false;
    for (const auto& s : m.structs) {
        if (!s) { error = "source generator: null AST struct"; return false; }
        for (const auto& r : s->requirements) if (!validateExpr(r.get(), error, "requires")) return false;
        for (const auto& x : s->members) if (!validateMember(x.get(), error)) return false;
    }
    for (const auto& e : m.enums) {
        if (e.underlying && !validateTypeRef(*e.underlying, error, "enum underlying type")) return false;
        for (const auto& x : e.items) if (!validateExpr(x.value.get(), error, "enum value")) return false;
    }
    for (const auto& c : m.constants) if (!validateExpr(c.expr.get(), error, "constant")) return false;
    for (const auto& p : m.parameters) if (!validateTypeRef(p.type, error, "parameter")) return false;
    return true;
}

} // namespace

bool generateSource(const ast::Module& module, std::string& output, std::string& error) {
    output.clear(); error.clear(); Writer w;
    if (!validateModule(module, error)) return false;
    if (!module.attributes.empty()) { error = "source generator cannot represent Module::attributes with the current grammar"; return false; }
    docs(w,module.documentation);
    if (!module.nameSpace.empty()) w.line("namespace "+module.nameSpace+";");
    for (const auto& [name,type] : module.attributeDecls) {
        const std::string t=attributeType(type);
        w.line("attribute "+name+(t.empty()?"":": "+t)+";");
    }
    if (auto e=endian(module.endian); !e.empty()) w.line("@ endian "+e);
    if (!module.order.empty()) {
        if (!module.attributeDecls.empty() || !module.nameSpace.empty() || module.endian != ast::Endian::Inherit || !module.documentation.empty()) w.line();
        for (const auto& r:module.order) { if (!generateTop(module,r,w,error)) { output.clear(); return false; } w.line(); }
    } else {
        // Hand-built ASTs may omit order. Refuse to invent an ordering.
        if (!module.aliases.empty() || !module.structs.empty() || !module.enums.empty() || !module.constants.empty() || !module.callbacks.empty() || !module.parameters.empty()) {
            error="source generator requires Module::order for deterministic top-level reconstruction"; output.clear(); return false;
        }
    }
    output=w.out.str();
    if (output.empty()) return true;
    while (output.size() >= 2 && output.compare(output.size()-2,2,"\n\n")==0) output.pop_back();
    return true;
}

} // namespace embx::codegen
