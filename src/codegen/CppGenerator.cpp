#include "codegen/CppGenerator.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <unordered_set>
namespace embx::codegen {
namespace {
std::string cppTypeName(std::string n) {
    if (n == "u8") return "std::uint8_t";
    if (n == "i8") return "std::int8_t";
    if (n == "u16") return "std::uint16_t";
    if (n == "i16") return "std::int16_t";
    if (n == "u32") return "std::uint32_t";
    if (n == "i32") return "std::int32_t";
    if (n == "u64") return "std::uint64_t";
    if (n == "i64") return "std::int64_t";
    if (n == "f32") return "float";
    if (n == "f64") return "double";
    return n.empty() ? "void" : n;
}
std::string leafName(const std::string& name) {
    const auto p = name.rfind("::");
    return p == std::string::npos ? name : name.substr(p + 2);
}
std::string qualify(const std::string& name) {
    return name.empty() ? std::string{} : (name.front() == ':' ? name : "::" + name);
}
std::string sanitizeIdentifier(const std::string& name) {
    std::string out;
    out.reserve(name.size() + 8);
    for (char c : name) out += (std::isalnum(static_cast<unsigned char>(c)) || c == '_') ? c : '_';
    if (out.empty() || std::isdigit(static_cast<unsigned char>(out.front()))) out = "_" + out;
    return out;
}
std::vector<std::string> splitNamespace(const std::string& name) {
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (start <= name.size()) {
        const std::size_t pos = name.find("::", start);
        const std::string part = name.substr(start, pos == std::string::npos ? std::string::npos : pos - start);
        if (!part.empty()) parts.push_back(part);
        if (pos == std::string::npos) break;
        start = pos + 2;
    }
    return parts;
}
std::string indent(int n) { return std::string(static_cast<std::size_t>(n) * 4u, ' '); }
void emitNamespaceOpen(std::ostringstream& os, const std::vector<std::string>& ns) {
    for (const auto& p : ns) os << "namespace " << p << " {\n";
}
void emitNamespaceClose(std::ostringstream& os, const std::vector<std::string>& ns) {
    for (auto it = ns.rbegin(); it != ns.rend(); ++it) os << "} // namespace " << *it << "\n";
}
bool hasDynamicSuffix(const plan::Type& t) {
    for (const auto& d : t.dimensions) if (d.kind == core::Dimension::Kind::Remaining) return true;
    return false;
}
const plan::Type* resolveAlias(const plan::Module& module, const plan::Type& input, std::string& error) {
    const plan::Type* t = &input;
    // bytes/string are built-in terminal types. They are represented as named
    // semantic types but do not have SymbolIds and therefore must not enter the
    // user-defined alias resolution path.
    if (t->kind == core::TypeKind::Bytes || t->kind == core::TypeKind::String) return t;
    std::unordered_set<core::SymbolId> seenIds;
    while (t->kind == core::TypeKind::Named) {
        const plan::Alias* a = nullptr;
        if (t->reference.valid()) {
            const auto it = module.aliasIndexBySymbol.find(t->reference.id);
            if (it != module.aliasIndexBySymbol.end()) a = &module.aliases[it->second];
            if (!seenIds.insert(t->reference.id).second) { error = "cyclic alias during C++ generation: " + t->name; return nullptr; }
        }
        if (!a) {
            if (module.structIndexBySymbol.find(t->reference.id) != module.structIndexBySymbol.end() ||
                module.enumIndexBySymbol.find(t->reference.id) != module.enumIndexBySymbol.end()) return t;
            error = "SymbolId does not identify a known plan type: " + t->name;
            return nullptr;
        }
        t = &a->target;
    }
    return t;
}

const plan::Enum* findEnum(const plan::Module& module, const plan::Type& type) {
    if (!type.reference.valid()) return nullptr;
    const auto it = module.enumIndexBySymbol.find(type.reference.id);
    return it == module.enumIndexBySymbol.end() ? nullptr : &module.enums[it->second];
}
const plan::Struct* findStruct(const plan::Module& module, const plan::Type& type) {
    if (!type.reference.valid()) return nullptr;
    const auto it = module.structIndexBySymbol.find(type.reference.id);
    return it == module.structIndexBySymbol.end() ? nullptr : &module.structs[it->second];
}
bool resolveEnumUnderlying(const plan::Module& module, const plan::Enum& e, std::string& base, std::string& error) {
    const plan::Type* t = &e.underlying;
    std::unordered_set<core::SymbolId> seen;
    while (t->kind == core::TypeKind::Named) {
        if (t->name == "") { error = "invalid enum underlying type: " + e.name; return false; }
        if (!t->reference.valid()) { error = "enum underlying type is unresolved: " + e.name; return false; }
        const auto ei = module.enumIndexBySymbol.find(t->reference.id);
        if (ei != module.enumIndexBySymbol.end()) { error = "enum underlying type cannot be an enum: " + e.name; return false; }
        const auto ai = module.aliasIndexBySymbol.find(t->reference.id);
        if (ai == module.aliasIndexBySymbol.end()) { error = "enum underlying type must resolve to a scalar integer: " + e.name; return false; }
        if (!seen.insert(t->reference.id).second) { error = "cyclic enum underlying alias: " + e.name; return false; }
        t = &module.aliases[ai->second].target;
    }
    const auto isInt = [](const std::string& n) {
        return n=="u8"||n=="i8"||n=="u16"||n=="i16"||n=="u32"||n=="i32"||n=="u64"||n=="i64";
    };
    if (!isInt(t->name)) { error = "enum underlying type must resolve to a scalar integer: " + e.name; return false; }
    base = t->name;
    return true;
}
std::string valueLiteral(const runtime::Value& v, const std::string& cppType, std::string& error) {
    std::ostringstream os;
    if (const auto* p = std::get_if<std::int64_t>(&v)) { os << *p; return os.str(); }
    if (const auto* p = std::get_if<std::uint64_t>(&v)) { os << *p << "ULL"; return os.str(); }
    if (const auto* p = std::get_if<double>(&v)) {
        if (!std::isfinite(*p)) { error = "cannot generate non-finite constant"; return {}; }
        os << std::setprecision(17) << *p;
        if (cppType == "float") os << 'f';
        return os.str();
    }
    if (const auto* p = std::get_if<bool>(&v)) return *p ? "true" : "false";
    error = "unsupported constant value";
    return {};
}
bool resolveFieldType(const plan::Module& module, const plan::Field& f, const plan::Type*& base, std::string& error) {
    base = resolveAlias(module, f.type, error);
    return base != nullptr;
}
std::string fieldExpr(const std::string& object, const std::string& field) {
    return object + "." + leafName(field);
}
std::string parameterFieldExpr(const plan::Module& module, const plan::Expr& e) {
    if (!e.reference.valid()) return {};
    const auto it = module.parameterIndexBySymbol.find(e.reference.id);
    if (it == module.parameterIndexBySymbol.end()) return {};
    return "embx_generated_params." + sanitizeIdentifier(module.parameters[it->second].name);
}
std::string emitCodeExpr(const plan::Module&, const plan::Struct&, const plan::Expr*, const std::string&, std::string&);

std::string generatedParameterSignature(const plan::Module& module) {
    return module.parameters.empty() ? std::string{} : ", const embx_generated_detail::RuntimeParameters& embx_generated_params";
}
std::string generatedParameterCall(const plan::Module& module) {
    return module.parameters.empty() ? std::string{} : ", embx_generated_params";
}

std::string cppStringLiteral(const std::string& value) {
    std::ostringstream os;
    os << "\"";
    for (unsigned char c : value) {
        switch (c) {
        case '\\': os << "\\\\"; break;
        case '"': os << "\\\""; break;
        case '\n': os << "\\n"; break;
        case '\r': os << "\\r"; break;
        case '\t': os << "\\t"; break;
        default:
            if (c < 0x20) { os << "\\x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(c) << std::dec << std::setfill(' '); }
            else os << static_cast<char>(c);
        }
    }
    os << "\"";
    return os.str();
}

std::string emitCallbackArg(const plan::Module& module, const plan::Struct& s, const plan::Expr* e, const std::string& object, std::string& error) {
    const std::string expr = emitCodeExpr(module, s, e, object, error);
    if (expr.empty()) return {};
    switch (e->type) {
    case core::ExprType::IntegerSigned: return "embx_generated_detail::CallbackValue(static_cast<std::int64_t>(" + expr + "))";
    case core::ExprType::IntegerUnsigned: return "embx_generated_detail::CallbackValue(static_cast<std::uint64_t>(" + expr + "))";
    case core::ExprType::Floating: return "embx_generated_detail::CallbackValue(static_cast<double>(" + expr + "))";
    case core::ExprType::Boolean: return "embx_generated_detail::CallbackValue(static_cast<bool>(" + expr + "))";
    default: error = "generated C++ callbacks support scalar expression arguments only: " + s.name; return {};
    }
}

bool emitCallbackArgs(std::ostringstream& c, const plan::Module& module, const plan::Struct& s, const plan::Callback& cb, const std::string& object, std::string& error) {
    c << "    std::vector<embx_generated_detail::CallbackValue> callbackArgs{";
    for (std::size_t i = 0; i < cb.args.size(); ++i) {
        if (i) c << ", ";
        const std::string arg = emitCallbackArg(module, s, cb.args[i].get(), object, error);
        if (arg.empty()) return false;
        c << arg;
    }
    c << "};\n";
    return true;
}

bool emitCallbackDecode(std::ostringstream& c, const plan::Module& module, const plan::Struct& s, const plan::Callback& cb, const std::string& object, std::string& error) {
    if (cb.direction != core::CallbackDirection::Decode) { error = "callback direction is not valid for decoder: " + cb.name; return false; }
    if (!emitCallbackArgs(c, module, s, cb, object, error)) return false;
    c << "    if(!callbacks){if(error)*error=\"generated decode callback registry is not configured\";return false;}\n"
      << "    if(!callbacks->callDecode(" << cppStringLiteral(cb.name) << ",r" << generatedParameterCall(module) << ",callbackArgs,error)){return false;}\n";
    return true;
}

bool emitCallbackEncode(std::ostringstream& c, const plan::Module& module, const plan::Struct& s, const plan::Callback& cb, const std::string& object, std::string& error) {
    if (cb.direction != core::CallbackDirection::Encode) { error = "callback direction is not valid for encoder: " + cb.name; return false; }
    if (!emitCallbackArgs(c, module, s, cb, object, error)) return false;
    c << "    if(!callbacks){if(error)*error=\"generated encode callback registry is not configured\";return false;}\n"
      << "    if(!callbacks->callEncode(" << cppStringLiteral(cb.name) << ",w" << generatedParameterCall(module) << ",callbackArgs,error)){return false;}\n";
    return true;
}

std::string emitRequireExpr(const plan::Module& module, const plan::Expr* e, const std::string& object, std::string& error) {
    if (!e) { error = "requires expression is missing"; return {}; }
    switch (e->kind) {
    case core::ExprKind::Literal:
        if (e->type == core::ExprType::IntegerSigned) return e->text;
        if (e->type == core::ExprType::IntegerUnsigned) return e->text + "ULL";
        if (e->type == core::ExprType::Floating) return e->text;
        if (e->type == core::ExprType::Boolean) return e->text;
        error = "generated C++ requires supports numeric and boolean literals only"; return {};
    case core::ExprKind::Identifier:
        if (e->text == "$size_in_bytes" || e->text == "$min_size_in_bytes" || e->text == "$max_size_in_bytes" || e->text == "$next") {
            error = "generated C++ requires does not support layout built-ins"; return {};
        }
        if (const auto parameter = parameterFieldExpr(module, *e); !parameter.empty()) return parameter;
        return fieldExpr(object, e->text);
    case core::ExprKind::Parenthesized: {
        std::string x = emitRequireExpr(module, e->left.get(), object, error);
        return x.empty() ? std::string{} : "(" + x + ")";
    }
    case core::ExprKind::Unary: {
        if (e->op != "-") { error = "unsupported unary operator in requires"; return {}; }
        std::string x = emitRequireExpr(module, e->right.get(), object, error);
        return x.empty() ? std::string{} : "(-" + x + ")";
    }
    case core::ExprKind::Binary: {
        std::string l = emitRequireExpr(module, e->left.get(), object, error); if (l.empty()) return {};
        std::string r = emitRequireExpr(module, e->right.get(), object, error); if (r.empty()) return {};
        static const std::unordered_set<std::string> ops = {"+","-","*","/","%","==","!=","<","<=",">",">=","&&","||"};
        if (!ops.count(e->op)) { error = "unsupported binary operator in requires: " + e->op; return {}; }
        return "(" + l + " " + e->op + " " + r + ")";
    }
    }
    error = "unknown expression kind in requires"; return {};
}
bool emitSizeExpr(const plan::Module& module, const plan::Expr* e, const std::string& object, std::string& out, std::string& error) {
    if (!e) { error = "missing dynamic size expression"; return false; }
    if (e->kind == core::ExprKind::Literal) {
        out = "static_cast<std::uint64_t>(" + e->text + "ULL)";
        return true;
    }
    if (e->kind == core::ExprKind::Identifier) {
        const std::string ref = parameterFieldExpr(module, *e);
        out = "embx_generated_detail::checkedSize(" + (ref.empty() ? fieldExpr(object, e->text) : ref) + ")";
        return true;
    }
    error = "generated C++ backend supports dynamic size expressions only as literals or field identifiers";
    return false;
}
bool emitOffsetExpr(const plan::Module& module, const plan::At& a, const std::string& object, std::string& out, std::string& error) {
    // Prefer materialized Plan layout when available. This is required for
    // built-in layout values such as $next, which have no source-field spelling
    // suitable for generated C++ lookup.
    if (a.staticOffset) { out = "static_cast<std::uint64_t>(" + std::to_string(*a.staticOffset) + "ULL)"; return true; }
    if (a.offset) return emitSizeExpr(module, a.offset.get(), object, out, error);
    error = "at operation is missing offset"; return false;
}
bool emitAlignExpr(const plan::Module& module, const plan::Align& a, const std::string& object, std::string& out, std::string& error) {
    if (a.alignment) return emitSizeExpr(module, a.alignment.get(), object, out, error);
    if (a.staticAlignment) { out = "static_cast<std::uint64_t>(" + std::to_string(*a.staticAlignment) + "ULL)"; return true; }
    error = "align operation is missing alignment"; return false;
}
bool emitBlockSizeExpr(const plan::Module& module, const plan::Block& b, const std::string& object, std::string& out, std::string& error) {
    if (b.size) return emitSizeExpr(module, b.size.get(), object, out, error);
    if (b.staticSize) {
        out = "static_cast<std::uint64_t>(" + std::to_string(*b.staticSize) + "ULL)";
        return true;
    }
    error = "block is missing size expression";
    return false;
}
std::string virtualCppType(core::ExprType t, std::string& error) {
    switch (t) {
    case core::ExprType::IntegerSigned: return "std::int64_t";
    case core::ExprType::IntegerUnsigned: return "std::uint64_t";
    case core::ExprType::Floating: return "double";
    case core::ExprType::Boolean: return "bool";
    default: error = "unsupported generated virtual field expression type"; return {};
    }
}
const plan::Field* findFieldBySymbol(const plan::Struct& s, core::SymbolId id) {
    for (const auto& op : s.members) {
        if (!op) continue;
        if (op->kind == plan::OpKind::Field) {
            const auto& f = static_cast<const plan::Field&>(*op);
            if (f.symbol == id) return &f;
        } else if (op->kind == plan::OpKind::At) {
            const auto& a = static_cast<const plan::At&>(*op);
            for (const auto& sub : a.members) if (sub && sub->kind == plan::OpKind::Field) {
                const auto& f = static_cast<const plan::Field&>(*sub);
                if (f.symbol == id) return &f;
            }
        } else if (op->kind == plan::OpKind::Conditional) {
            const auto& c = static_cast<const plan::Conditional&>(*op);
            const auto walk = [&](const auto& self, const std::vector<std::unique_ptr<plan::Op>>& ms) -> const plan::Field* {
                for (const auto& x : ms) {
                    if (!x) continue;
                    if (x->kind == plan::OpKind::Field) {
                        const auto& f = static_cast<const plan::Field&>(*x);
                        if (f.symbol == id) return &f;
                    } else if (x->kind == plan::OpKind::Conditional) {
                        const auto& n = static_cast<const plan::Conditional&>(*x);
                        if (auto* r = self(self, n.thenMembers)) return r;
                        if (auto* r = self(self, n.elseMembers)) return r;
                    }
                }
                return nullptr;
            };
            if (auto* r = walk(walk, c.thenMembers)) return r;
            if (auto* r = walk(walk, c.elseMembers)) return r;
        }
    }
    return nullptr;
}
const plan::FieldAlias* findFieldAliasBySymbol(const plan::Struct& s, core::SymbolId id) {
    for (const auto& op : s.members) if (op && op->kind == plan::OpKind::Alias) {
        const auto& a = static_cast<const plan::FieldAlias&>(*op);
        if (a.symbol == id) return &a;
    }
    return nullptr;
}
std::string emitCodeExpr(const plan::Module& module, const plan::Struct& s, const plan::Expr* e, const std::string& object, std::string& error) {
    if (!e) { error = "missing generated expression"; return {}; }
    switch (e->kind) {
    case core::ExprKind::Literal:
        if (e->type == core::ExprType::Boolean || e->type == core::ExprType::IntegerSigned || e->type == core::ExprType::IntegerUnsigned || e->type == core::ExprType::Floating) return e->text + (e->type == core::ExprType::IntegerUnsigned ? "ULL" : "");
        error = "unsupported generated expression literal"; return {};
    case core::ExprKind::Identifier: {
        if (e->reference.valid()) {
            const auto* sym = module.symbolTable.find(e->reference.id);
            if (sym) {
                if (sym->kind == core::SymbolKind::Constant) {
                    const auto it = module.constantIndexBySymbol.find(e->reference.id);
                    if (it != module.constantIndexBySymbol.end()) return qualify(module.constants[it->second].name);
                }
                if (sym->kind == core::SymbolKind::EnumItem) return qualify(sym->name);
                if (sym->kind == core::SymbolKind::AliasField) {
                    if (const auto* a = findFieldAliasBySymbol(s, e->reference.id)) return fieldExpr(object, a->targetName);
                }
                if (sym->kind == core::SymbolKind::Parameter) {
                    if (const auto parameter = parameterFieldExpr(module, *e); !parameter.empty()) return parameter;
                }
                return fieldExpr(object, sym->name);
            }
        }
        if (const auto parameter = parameterFieldExpr(module, *e); !parameter.empty()) return parameter;
        return fieldExpr(object, e->text);
    }
    case core::ExprKind::Parenthesized: {
        auto x = emitCodeExpr(module, s, e->left.get(), object, error); return x.empty() ? std::string{} : "(" + x + ")";
    }
    case core::ExprKind::Unary: {
        if (e->op != "-") { error = "unsupported generated unary operator: " + e->op; return {}; }
        auto x = emitCodeExpr(module, s, e->right.get(), object, error); return x.empty() ? std::string{} : "(-" + x + ")";
    }
    case core::ExprKind::Binary: {
        auto l = emitCodeExpr(module, s, e->left.get(), object, error); if (l.empty()) return {};
        auto r = emitCodeExpr(module, s, e->right.get(), object, error); if (r.empty()) return {};
        static const std::unordered_set<std::string> ops={"+","-","*","/","%","==","!=","<","<=",">",">=","&&","||"};
        if (!ops.count(e->op)) { error = "unsupported generated binary operator: " + e->op; return {}; }
        return "(" + l + " " + e->op + " " + r + ")";
    }
    }
    error = "unknown generated expression kind"; return {};
}

std::string scalarDecodeName(const plan::Field& f, const std::string& n) {
    const std::string b = f.endian == plan::Endian::Big ? "true" : f.endian == plan::Endian::Native ? "embx_generated_detail::hostLittle() ? false : true" : "false";
    std::string raw;
    if (n == "u8" || n == "u16" || n == "u32" || n == "u64") raw = "r.getUnsigned(" + std::to_string(std::stoul(n.substr(1))/8u) + "," + b + ")";
    else if (n == "i8" || n == "i16" || n == "i32" || n == "i64") raw = "r.getSigned(" + std::to_string(std::stoul(n.substr(1))/8u) + "," + b + ")";
    else if (n == "f32") raw = "r.getF32(" + b + ")";
    else if (n == "f64") raw = "r.getF64(" + b + ")";
    else return {};
    if (f.transform) return "((" + raw + ") * " + std::to_string(f.transform->factor) + ")";
    return raw;
}
std::string scalarDecode(const plan::Field& f, const plan::Type& t) { return scalarDecodeName(f, t.name); }
std::string scalarEncodeName(const plan::Field& f, const std::string& n, const std::string& value) {
    const std::string b = f.endian == plan::Endian::Big ? "true" : f.endian == plan::Endian::Native ? "embx_generated_detail::hostLittle() ? false : true" : "false";
    std::string wire = value;
    if (f.transform) wire = "(" + value + " / " + std::to_string(f.transform->factor) + ")";
    if (n == "u8" || n == "u16" || n == "u32" || n == "u64") return "w.putUnsigned(static_cast<std::uint64_t>(" + wire + ")," + std::to_string(std::stoul(n.substr(1))/8u) + "," + b + ");";
    if (n == "i8" || n == "i16" || n == "i32" || n == "i64") return "w.putSigned(static_cast<std::int64_t>(" + wire + ")," + std::to_string(std::stoul(n.substr(1))/8u) + "," + b + ");";
    if (n == "f32") return "w.putF32(static_cast<float>(" + wire + ")," + b + ");";
    if (n == "f64") return "w.putF64(" + wire + "," + b + ");";
    return {};
}
std::string scalarEncode(const plan::Field& f, const plan::Type& t, const std::string& value) { return scalarEncodeName(f, t.name, value); }
std::optional<std::uint64_t> staticDimensionExtent(const core::Dimension& d) {
    if (d.kind != core::Dimension::Kind::Fixed || !d.expression) return std::nullopt;
    if (d.expression->kind != core::ExprKind::Literal) return std::nullopt;
    try {
        if (d.expression->type == core::ExprType::IntegerSigned) {
            const auto v = std::stoll(d.expression->text);
            if (v < 0) return std::nullopt;
            return static_cast<std::uint64_t>(v);
        }
        if (d.expression->type == core::ExprType::IntegerUnsigned) return std::stoull(d.expression->text);
    } catch (...) {}
    return std::nullopt;
}
std::string cppFieldElementType(const plan::Module&, const plan::Field& f,
                                const plan::Type& base, std::string& error) {
    if (base.kind == core::TypeKind::Bytes) return "std::uint8_t";
    if (base.kind == core::TypeKind::String) return "char";
    if (f.type.kind == core::TypeKind::Named) return qualify(f.type.name);
    if (base.kind == core::TypeKind::Primitive) return cppTypeName(base.name);
    if (base.kind == core::TypeKind::Named) return qualify(base.name);
    error = "unsupported generated field type: " + f.name;
    return {};
}
std::string cppFieldType(const plan::Module& module, const plan::Field& f, std::string& error) {
    const plan::Type* base = nullptr;
    if (!resolveFieldType(module, f, base, error)) return {};
    if (f.transform && f.type.dimensions.empty() && base->kind == core::TypeKind::Primitive) return "double";
    if (!f.type.terminator.empty()) {
        if (f.type.dimensions.size() == 1 && f.type.dimensions[0].kind == core::Dimension::Kind::Remaining) {
            std::string type = cppFieldElementType(module, f, *base, error);
            return type.empty() ? std::string{} : "std::vector<" + type + ">";
        }
        if (base->kind != core::TypeKind::Bytes) { error = "generated terminated sequence requires bytes or [*] element sequence: " + f.name; return {}; }
        return "std::vector<std::uint8_t>";
    }
    if (f.type.dimensions.empty()) return cppFieldElementType(module, f, *base, error);
    if ((base->kind == core::TypeKind::Bytes || base->kind == core::TypeKind::String) && f.type.dimensions.size() == 1 && f.type.dimensions[0].kind == core::Dimension::Kind::Remaining)
        return base->kind == core::TypeKind::Bytes ? "std::vector<std::uint8_t>" : "std::string";
    std::string type = cppFieldElementType(module, f, *base, error);
    if (type.empty()) return {};
    for (auto it = f.type.dimensions.rbegin(); it != f.type.dimensions.rend(); ++it) {
        if (const auto extent = staticDimensionExtent(*it)) type = "std::array<" + type + ", " + std::to_string(*extent) + ">";
        else type = "std::vector<" + type + ">";
    }
    return type;
}
void emitCodecHelpers(std::ostringstream& h, const plan::Module& module) {
    h << "namespace embx_generated_detail {\n";
    if (!module.parameters.empty()) {
        h << "struct RuntimeParameters {\n";
        for (const auto& parameter : module.parameters) {
            if (parameter.type.kind != core::TypeKind::Primitive) {
                return;
            }
            h << "    " << cppTypeName(parameter.type.name) << " "
              << sanitizeIdentifier(parameter.name) << "{};\n";
        }
        h << "};\n";
    }
    h << "inline bool hostLittle() { const std::uint16_t v=1; return *reinterpret_cast<const std::uint8_t*>(&v)==1; }\n"
      << "struct Reader {\n"
      << "    const std::vector<std::uint8_t>& d; std::size_t p=0; std::size_t begin=0; std::size_t limit=0; bool ok=true; std::string error;\n"
      << "    explicit Reader(const std::vector<std::uint8_t>& x):d(x),begin(0),limit(x.size()){}\n"
      << "    Reader(const std::vector<std::uint8_t>& x,std::size_t b,std::size_t n):d(x),p(b),begin(b),limit(b+n){}\n"
      << "    std::size_t remaining() const { return p <= limit ? limit-p : 0; }\n"
      << "    bool need(std::size_t n){ if(!ok || p>limit || n>limit-p){ok=false;error=\"input truncated\";return false;} return true;}\n"
      << "    std::uint64_t getUnsigned(std::size_t n,bool big){if(!need(n)||n>8)return 0;std::uint64_t v=0;if(big){for(std::size_t i=0;i<n;i++)v=(v<<8)|d[p+i];}else{for(std::size_t i=0;i<n;i++)v|=std::uint64_t(d[p+i])<<(8*i);}p+=n;return v;}\n"
      << "    std::int64_t getSigned(std::size_t n,bool big){std::uint64_t u=getUnsigned(n,big);if(!ok)return 0;if(n<8&&n>0&&(u&(std::uint64_t(1)<<(n*8-1))))u|=~((std::uint64_t(1)<<(n*8))-1);return static_cast<std::int64_t>(u);}\n"
      << "    float getF32(bool big){auto u=static_cast<std::uint32_t>(getUnsigned(4,big));float v{};std::memcpy(&v,&u,4);return v;}\n"
      << "    double getF64(bool big){auto u=getUnsigned(8,big);double v{};std::memcpy(&v,&u,8);return v;}\n"
      << "    void getBytes(std::vector<std::uint8_t>& out,std::size_t n){if(!need(n))return;out.assign(d.begin()+static_cast<std::ptrdiff_t>(p),d.begin()+static_cast<std::ptrdiff_t>(p+n));p+=n;}\n"
      << "    void getString(std::string& out,std::size_t n){if(!need(n))return;out.assign(reinterpret_cast<const char*>(d.data()+p),n);p+=n;}\n"
      << "    bool getTerminatedBytes(std::vector<std::uint8_t>& out,const std::vector<std::uint8_t>& term,std::uint64_t maxPayload,std::string* error){if(term.empty()){if(error)*error=\"empty terminator\";return false;}if(maxPayload>static_cast<std::uint64_t>((std::numeric_limits<std::size_t>::max)()-term.size())){if(error)*error=\"terminated sequence maximum exceeds host size\";return false;}const std::size_t saved=p;const std::size_t limitN=static_cast<std::size_t>(maxPayload)+term.size();out.clear();for(std::size_t i=0;i<limitN&&remaining()!=0;++i){const auto b=d[p++];out.push_back(b);if(out.size()>=term.size()&&std::equal(term.rbegin(),term.rend(),out.rbegin())){out.resize(out.size()-term.size());return true;}}p=saved;ok=false;this->error=\"terminated sequence terminator not found within maximum payload length\";if(error)*error=this->error;return false;}\n"
      << "    bool atTerm(const std::vector<std::uint8_t>& term) const { return !term.empty() && remaining() >= term.size() && std::equal(term.begin(),term.end(),d.begin()+static_cast<std::ptrdiff_t>(p)); }\n"
      << "    void consumeTerm(const std::vector<std::uint8_t>& term) { p += term.size(); }\n"
      << "};\n"
      << "template<class T> std::uint64_t checkedSize(T v){if constexpr(std::is_signed_v<T>){if(v<0)throw std::runtime_error(\"negative dynamic size\");}return static_cast<std::uint64_t>(v);}\n"
      << "inline bool checkedHostSize(std::uint64_t v,std::size_t& out){if(v>static_cast<std::uint64_t>((std::numeric_limits<std::size_t>::max)()))return false;out=static_cast<std::size_t>(v);return true;}\n"
      << "struct Writer { std::vector<std::uint8_t>& d; std::size_t p=0; explicit Writer(std::vector<std::uint8_t>& x):d(x){}\n"
      << " bool seek(std::size_t n){p=n;if(d.size()<p)d.resize(p,0);return true;}\n"
      << " bool ensure(std::size_t n){if(n>std::numeric_limits<std::size_t>::max()-p)return false;const auto e=p+n;if(d.size()<e)d.resize(e,0);return true;}\n"
      << " void putUnsigned(std::uint64_t v,std::size_t n,bool big){if(!ensure(n))throw std::runtime_error(\"output size overflow\");if(big){for(std::size_t i=0;i<n;i++)d[p+i]=static_cast<std::uint8_t>(v>>(8*(n-1-i)));}else{for(std::size_t i=0;i<n;i++)d[p+i]=static_cast<std::uint8_t>(v>>(8*i));}p+=n;}\n"
      << " void putSigned(std::int64_t v,std::size_t n,bool big){putUnsigned(static_cast<std::uint64_t>(v),n,big);}\n"
      << " void putF32(float v,bool big){std::uint32_t u;std::memcpy(&u,&v,4);putUnsigned(u,4,big);}\n"
      << " void putF64(double v,bool big){std::uint64_t u;std::memcpy(&u,&v,8);putUnsigned(u,8,big);}\n"
      << " void putBytes(const std::vector<std::uint8_t>& v){if(!ensure(v.size()))throw std::runtime_error(\"output size overflow\");std::copy(v.begin(),v.end(),d.begin()+static_cast<std::ptrdiff_t>(p));p+=v.size();}\n"
      << " void putString(const std::string& v){if(!ensure(v.size()))throw std::runtime_error(\"output size overflow\");std::copy(v.begin(),v.end(),d.begin()+static_cast<std::ptrdiff_t>(p));p+=v.size();}\n"
      << " void putTerminatedBytes(const std::vector<std::uint8_t>& v,const std::vector<std::uint8_t>& term,std::uint64_t maxPayload){if(term.empty())throw std::runtime_error(\"empty terminator\");if(static_cast<std::uint64_t>(v.size())>maxPayload)throw std::runtime_error(\"terminated sequence payload exceeds maximum length\");if(std::search(v.begin(),v.end(),term.begin(),term.end())!=v.end())throw std::runtime_error(\"terminated sequence payload contains its terminator\");putBytes(v);putBytes(term);}\n"
      << "};\n"
      << "using CallbackValue = std::variant<std::int64_t,std::uint64_t,double,bool>;\n"
      << "using DecodeCallback = std::function<bool(const std::string&,Reader&," << (module.parameters.empty() ? std::string("const std::nullptr_t&") : std::string("const RuntimeParameters&")) << ",const std::vector<CallbackValue>&,std::string&)>;\n"
      << "using EncodeCallback = std::function<bool(const std::string&,Writer&," << (module.parameters.empty() ? std::string("const std::nullptr_t&") : std::string("const RuntimeParameters&")) << ",const std::vector<CallbackValue>&,std::string&)>;\n"
      << "struct Callbacks { std::unordered_map<std::string,DecodeCallback> decode; std::unordered_map<std::string,EncodeCallback> encode;\n"
      << " bool addDecode(const std::string& n,DecodeCallback f){if(n.empty()||!f)return false;return decode.emplace(n,std::move(f)).second;}\n"
      << " bool addEncode(const std::string& n,EncodeCallback f){if(n.empty()||!f)return false;return encode.emplace(n,std::move(f)).second;}\n"
      << " bool callDecode(const std::string& n,Reader& r," << (module.parameters.empty() ? std::string("const std::nullptr_t&") : std::string("const RuntimeParameters&")) << ",const std::vector<CallbackValue>& a,std::string* error) const {auto it=decode.find(n);if(it==decode.end()){if(error)*error=\"callback not registered: \"+n;return false;} const auto p=r.p; const bool ok=r.ok; const auto e=r.error; try {std::string msg; if(!it->second(n,r," << (module.parameters.empty() ? std::string("nullptr") : std::string("embx_generated_params")) << ",a,msg)){r.p=p;r.ok=ok;r.error=e;if(error)*error=msg.empty()?\"generated decode callback failed\":msg;return false;} return true;} catch(const std::exception& ex){r.p=p;r.ok=ok;r.error=e;if(error)*error=ex.what();return false;} catch(...){r.p=p;r.ok=ok;r.error=e;if(error)*error=\"generated decode callback threw\";return false;} }\n"
      << " bool callEncode(const std::string& n,Writer& w," << (module.parameters.empty() ? std::string("const std::nullptr_t&") : std::string("const RuntimeParameters&")) << ",const std::vector<CallbackValue>& a,std::string* error) const {auto it=encode.find(n);if(it==encode.end()){if(error)*error=\"callback not registered: \"+n;return false;} const auto p=w.p; const auto sz=w.d.size(); try {std::string msg; if(!it->second(n,w," << (module.parameters.empty() ? std::string("nullptr") : std::string("embx_generated_params")) << ",a,msg)){w.p=p;w.d.resize(sz);if(error)*error=msg.empty()?\"generated encode callback failed\":msg;return false;} if(w.p>w.d.size()){w.p=p;w.d.resize(sz);if(error)*error=\"generated encode callback advanced beyond output\";return false;} return true;} catch(const std::exception& ex){w.p=p;w.d.resize(sz);if(error)*error=ex.what();return false;} catch(...){w.p=p;w.d.resize(sz);if(error)*error=\"generated encode callback threw\";return false;} }\n"
      << "};\n"
      << "}\n\n";
}


bool collectStructDeps(const plan::Module& module, const plan::Type& type, std::unordered_set<core::SymbolId>& deps,
                       std::string& error) {
    const plan::Type* base = resolveAlias(module, type, error);
    if (!base) return false;
    if (base->kind != core::TypeKind::Named) return true;
    if (base->reference.valid()) {
        if (module.structIndexBySymbol.find(base->reference.id) != module.structIndexBySymbol.end()) deps.insert(base->reference.id);
        return true;
    }
    if (!base->reference.valid()) { error = "named struct dependency has no SymbolId: " + base->name; return false; }
    return true;
}
bool collectStructDepsMembers(const plan::Module& module, const plan::Struct& s, std::unordered_set<core::SymbolId>& deps, std::string& error) {
    std::function<bool(const std::vector<std::unique_ptr<plan::Op>>&)> walk = [&](const auto& ops) {
        for (const auto& op : ops) {
            if (!op) { error = "null operation in struct " + s.name; return false; }
            if (op->kind == plan::OpKind::Field) {
                const auto& f = static_cast<const plan::Field&>(*op);
                if (!collectStructDeps(module, f.type, deps, error)) return false;
            } else if (op->kind == plan::OpKind::Block || op->kind == plan::OpKind::At) {
                const auto& nested = (op->kind == plan::OpKind::Block)
                    ? static_cast<const plan::Block&>(*op).members
                    : static_cast<const plan::At&>(*op).members;
                if (!walk(nested)) return false;
            } else if (op->kind == plan::OpKind::Variant) {
                const auto& v = static_cast<const plan::Variant&>(*op);
                for (const auto& c : v.cases) {
                    if (c.type && !collectStructDeps(module, *c.type, deps, error)) return false;
                    if (!walk(c.members)) return false;
                }
                if (v.defaultType && !collectStructDeps(module, *v.defaultType, deps, error)) return false;
                if (!walk(v.defaultMembers)) return false;
            }
        }
        return true;
    };
    return walk(s.members);
}
bool topoVisitStruct(const plan::Module& module, core::SymbolId symbol,
                     std::unordered_set<core::SymbolId>& visiting, std::unordered_set<core::SymbolId>& done,
                     std::vector<std::size_t>& order, std::string& error) {
    if (done.count(symbol)) return true;
    const auto it = module.structIndexBySymbol.find(symbol);
    if (it == module.structIndexBySymbol.end()) { error = "unknown struct dependency SymbolId"; return false; }
    const std::size_t structIndex = it->second;
    const auto& name = module.structs[structIndex].name;
    if (!visiting.insert(symbol).second) { error = "cyclic by-value struct dependency in generated C++: " + name; return false; }
    std::unordered_set<core::SymbolId> deps;
    if (!collectStructDepsMembers(module, module.structs[structIndex], deps, error)) return false;
    for (const auto dep : deps) {
        if (dep == symbol) { error = "recursive by-value struct is not representable in generated C++: " + name; return false; }
        if (!topoVisitStruct(module, dep, visiting, done, order, error)) return false;
    }
    visiting.erase(symbol);
    done.insert(symbol);
    order.push_back(structIndex);
    return true;
}
bool emitDecodeField(std::ostringstream& c, const plan::Module& module, const plan::Field& f, const std::string& obj, std::string& error);
bool emitEncodeField(std::ostringstream& c, const plan::Module& module, const plan::Field& f, const std::string& obj, std::string& error);
bool emitBitsDecode(std::ostringstream& c, const plan::Module&, const plan::Bits& b, const std::string& obj, std::string& error);
bool emitBitsEncode(std::ostringstream& c, const plan::Module&, const plan::Bits& b, const std::string& obj, std::string& error);
bool emitStructType(std::ostringstream& h, const plan::Module& module, const plan::Struct& s, std::string& error);

std::string cppVariantScalarType(const plan::Module& module, const plan::Type& t, std::string& error) {
    const plan::Type* base = resolveAlias(module, t, error);
    if (!base) return {};
    if (!base->dimensions.empty()) { error = "generated C++ variant typed cases with dimensions are not yet supported"; return {}; }
    if (base->kind == core::TypeKind::Primitive) return cppTypeName(base->name);
    if (base->kind == core::TypeKind::Bytes) return "std::uint8_t";
    if (base->kind == core::TypeKind::String) return "char";
    if (base->kind == core::TypeKind::Named) {
        if (findStruct(module, *base) || findEnum(module, *base)) return qualify(base->name);
    }
    error = "unsupported generated C++ variant case type: " + base->name;
    return {};
}

std::string variantTypeName(const plan::Struct& parent, const plan::Variant& v) {
    return leafName(parent.name) + "_" + sanitizeIdentifier(v.name);
}
std::string variantCaseTypeName(const plan::Struct& parent, const plan::Variant& v, std::size_t index) {
    return variantTypeName(parent, v) + "_case" + std::to_string(index);
}

bool emitVariantCaseMembersType(std::ostringstream& h, const plan::Module& module,
                                const plan::Struct& parent, const plan::Variant& v,
                                std::size_t index, const std::vector<std::unique_ptr<plan::Op>>& members,
                                std::string& error) {
    const auto ns = splitNamespace(parent.name);
    std::vector<std::string> p(ns.begin(), ns.end() - (ns.empty() ? 0 : 1));
    emitNamespaceOpen(h, p);
    h << "struct " << variantCaseTypeName(parent, v, index) << " {\n";
    for (const auto& op : members) {
        if (!op) { error = "null operation in generated variant case"; return false; }
        if (op->kind == plan::OpKind::Field) {
            const auto& f = static_cast<const plan::Field&>(*op);
            std::string ft = cppFieldType(module, f, error);
            if (ft.empty()) return false;
            h << indent(1) << ft << " " << leafName(f.name) << "{};\n";
        } else if (op->kind == plan::OpKind::Bits) {
            const auto& b = static_cast<const plan::Bits&>(*op);
            for (const auto& bf : b.fields) {
                std::string ft = bf.type.kind == core::TypeKind::Primitive ? cppTypeName(bf.type.name) :
                                 bf.type.kind == core::TypeKind::Named ? qualify(bf.type.name) : "";
                if (ft.empty()) { error = "unsupported generated variant bits field type: " + bf.name; return false; }
                h << indent(1) << ft << " " << leafName(bf.name) << "{};\n";
            }
        } else { error = "generated C++ variant inline cases support fields and bits only"; return false; }
    }
    h << "};\n\n";
    emitNamespaceClose(h, p); h << "\n";
    return true;
}

bool emitVariantType(std::ostringstream& h, const plan::Module& module, const plan::Struct& parent,
                     const plan::Variant& v, std::string& error) {
    const auto ns = splitNamespace(parent.name);
    std::vector<std::string> p(ns.begin(), ns.end() - (ns.empty() ? 0 : 1));
    for (std::size_t i = 0; i < v.cases.size(); ++i) {
        if (!v.cases[i].type && !emitVariantCaseMembersType(h, module, parent, v, i, v.cases[i].members, error)) return false;
    }
    if (v.hasDefault && !v.defaultType && !emitVariantCaseMembersType(h, module, parent, v, v.cases.size(), v.defaultMembers, error)) return false;
    emitNamespaceOpen(h, p);
    const std::string type = variantTypeName(parent, v);
    h << "struct " << type << " {\n";
    h << indent(1) << "using storage = std::variant<";
    bool first = true;
    for (std::size_t i = 0; i < v.cases.size(); ++i) {
        if (!first) h << ", ";
        first = false;
        if (v.cases[i].type) {
            std::string ct = cppVariantScalarType(module, *v.cases[i].type, error); if (ct.empty()) return false;
            h << ct;
        } else h << variantCaseTypeName(parent, v, i);
    }
    if (v.hasDefault) {
        if (!first) h << ", ";
        if (v.defaultType) { std::string ct = cppVariantScalarType(module, *v.defaultType, error); if (ct.empty()) return false; h << ct; }
        else h << variantCaseTypeName(parent, v, v.cases.size());
    }
    h << ">;\n" << indent(1) << "storage value{};\n" << "};\n\n";
    emitNamespaceClose(h, p); h << "\n";
    return true;
}

bool emitVariantDecode(std::ostringstream& c, const plan::Module& module,
                       const plan::Variant& v, const std::string& obj, std::string& error) {
    std::string disc = emitRequireExpr(module, v.discriminator.get(), obj, error); if (disc.empty()) return false;
    const std::string dst = fieldExpr(obj, v.name);
    bool first = true;
    for (std::size_t i = 0; i < v.cases.size(); ++i) {
        std::string tag = valueLiteral(v.cases[i].tag, "std::uint64_t", error); if (tag.empty() && !error.empty()) return false;
        c << "    " << (first ? "if" : "else if") << " (" << disc << " == " << tag << ") {\n";
        first = false;
        if (v.cases[i].type) {
            std::string ct = cppVariantScalarType(module, *v.cases[i].type, error); if (ct.empty()) return false;
            c << "        " << dst << ".value.template emplace<" << i << ">();\n";
            plan::Field f; f.name = "value"; f.type = *v.cases[i].type;
            if (!emitDecodeField(c, module, f, dst + ".value", error)) return false;
        } else {
            c << "        " << dst << ".value.template emplace<" << i << ">();\n";
            const std::string target = dst + ".value.template get<" + std::to_string(i) + ">()";
            for (const auto& op : v.cases[i].members) {
                if (op->kind == plan::OpKind::Field) { if (!emitDecodeField(c, module, static_cast<const plan::Field&>(*op), target, error)) return false; }
                else if (op->kind == plan::OpKind::Bits) { if (!emitBitsDecode(c, module, static_cast<const plan::Bits&>(*op), target, error)) return false; }
                else { error = "generated C++ variant inline decode supports fields and bits only"; return false; }
            }
        }
        c << "    }\n";
    }
    if (v.hasDefault) {
        c << "    else {\n";
        const std::size_t i = v.cases.size();
        c << "        " << dst << ".value.template emplace<" << i << ">();\n";
        if (v.defaultType) {
            plan::Field f; f.name = "value"; f.type = *v.defaultType;
            if (!emitDecodeField(c, module, f, dst + ".value", error)) return false;
        } else {
            const std::string target = dst + ".value.template get<" + std::to_string(i) + ">()";
            for (const auto& op : v.defaultMembers) {
                if (op->kind == plan::OpKind::Field) { if (!emitDecodeField(c, module, static_cast<const plan::Field&>(*op), target, error)) return false; }
                else if (op->kind == plan::OpKind::Bits) { if (!emitBitsDecode(c, module, static_cast<const plan::Bits&>(*op), target, error)) return false; }
                else { error = "generated C++ variant default decode supports fields and bits only"; return false; }
            }
        }
        c << "    }\n";
    } else c << "    else { if(error)*error=\"no variant case matched\"; return false; }\n";
    return true;
}

bool emitVariantEncode(std::ostringstream& c, const plan::Module& module,
                       const plan::Variant& v, const std::string& obj, std::string& error) {
    std::string disc = emitRequireExpr(module, v.discriminator.get(), obj, error); if (disc.empty()) return false;
    const std::string src = fieldExpr(obj, v.name);
    bool first = true;
    for (std::size_t i = 0; i < v.cases.size(); ++i) {
        std::string tag = valueLiteral(v.cases[i].tag, "std::uint64_t", error); if (tag.empty() && !error.empty()) return false;
        c << "    " << (first ? "if" : "else if") << " (" << disc << " == " << tag << ") {\n"; first = false;
        c << "        if(" << src << ".value.index() != " << i << ") { if(error)*error=\"variant payload does not match discriminator\"; return false; }\n";
        const std::string value = src + ".value.template get<" + std::to_string(i) + ">()";
        if (v.cases[i].type) { plan::Field f; f.name="value"; f.type=*v.cases[i].type; if(!emitEncodeField(c,module,f,value,error)) return false; }
        else for (const auto& op : v.cases[i].members) {
            if (op->kind == plan::OpKind::Field) { if(!emitEncodeField(c,module,static_cast<const plan::Field&>(*op),value,error)) return false; }
            else if (op->kind == plan::OpKind::Bits) { if(!emitBitsEncode(c,module,static_cast<const plan::Bits&>(*op),value,error)) return false; }
            else { error="generated C++ variant inline encode supports fields and bits only"; return false; }
        }
        c << "    }\n";
    }
    if (v.hasDefault) {
        const std::size_t i=v.cases.size(); c<<"    else {\n";
        c<<"        if("<<src<<".value.index() != "<<i<<") { if(error)*error=\"variant payload does not match default discriminator\"; return false; }\n";
        const std::string value=src+".value.template get<"+std::to_string(i)+">()";
        if(v.defaultType){plan::Field f;f.name="value";f.type=*v.defaultType;if(!emitEncodeField(c,module,f,value,error))return false;}
        else for(const auto& op:v.defaultMembers){if(op->kind==plan::OpKind::Field){if(!emitEncodeField(c,module,static_cast<const plan::Field&>(*op),value,error))return false;}else if(op->kind==plan::OpKind::Bits){if(!emitBitsEncode(c,module,static_cast<const plan::Bits&>(*op),value,error))return false;}else{error="generated C++ variant default encode supports fields and bits only";return false;}}
        c<<"    }\n";
    } else c<<"    else { if(error)*error=\"no variant case matched\"; return false; }\n";
    return true;
}

bool emitBlockType(std::ostringstream& h, const plan::Module& module, const plan::Struct& parent, const plan::Block& b, std::string& error) {
    const auto ns = splitNamespace(parent.name);
    std::vector<std::string> parentNs(ns.begin(), ns.end() - (ns.empty() ? 0 : 1));
    const std::string type = leafName(parent.name) + "_" + b.name;
    emitNamespaceOpen(h, parentNs);
    h << "struct " << type << " {\n";
    for (const auto& op : b.members) {
        if (!op) { error = "null block operation"; return false; }
        if (op->kind != plan::OpKind::Field && op->kind != plan::OpKind::Bits && op->kind != plan::OpKind::Align) { error = "generated blocks support fields, bits and alignment: " + b.name; return false; }
        if (op->kind == plan::OpKind::Align) continue;
        if (op->kind == plan::OpKind::Field) {
            const auto& f = static_cast<const plan::Field&>(*op);
            std::string ft = cppFieldType(module, f, error);
            if (ft.empty()) return false;
            h << indent(1) << ft << " " << leafName(f.name) << "{};\n";
        } else {
            const auto& bits = static_cast<const plan::Bits&>(*op);
            for (const auto& bf : bits.fields) {
                std::string ft = bf.type.kind == core::TypeKind::Primitive ? cppTypeName(bf.type.name) :
                                 bf.type.kind == core::TypeKind::Named ? qualify(bf.type.name) : "";
                if (ft.empty()) { error = "unsupported generated block bits field type: " + bf.name; return false; }
                h << indent(1) << ft << " " << leafName(bf.name) << "{};\n";
            }
        }
    }
    h << "};\n\n";
    emitNamespaceClose(h, parentNs); h << "\n";
    return true;
}
bool emitConditionalFields(std::ostringstream& h, const plan::Module& module,
                           const std::vector<std::unique_ptr<plan::Op>>& members,
                           std::string& error) {
    for (const auto& op : members) {
        if (!op) { error = "null operation in conditional"; return false; }
        if (op->kind == plan::OpKind::Field) {
            const auto& f = static_cast<const plan::Field&>(*op);
            std::string ft = cppFieldType(module, f, error);
            if (ft.empty()) return false;
            h << indent(1) << ft << " " << leafName(f.name) << "{};\n";
        } else if (op->kind == plan::OpKind::Bits) {
            const auto& b = static_cast<const plan::Bits&>(*op);
            for (const auto& bf : b.fields) {
                std::string ft = bf.type.kind == core::TypeKind::Primitive ? cppTypeName(bf.type.name) :
                                 bf.type.kind == core::TypeKind::Named ? qualify(bf.type.name) : "";
                if (ft.empty()) { error = "unsupported generated conditional bits field type: " + bf.name; return false; }
                h << indent(1) << ft << " " << leafName(bf.name) << "{};\n";
            }
        } else if (op->kind == plan::OpKind::Virtual) {
            const auto& v = static_cast<const plan::Virtual&>(*op);
            std::string vt = virtualCppType(v.expression ? v.expression->type : core::ExprType::Invalid, error); if (vt.empty()) return false;
            h << indent(1) << vt << " " << leafName(v.name) << "{};\n";
        } else if (op->kind == plan::OpKind::Alias) {
            error = "generated conditional alias fields are not yet supported"; return false;
        } else if (op->kind == plan::OpKind::Conditional) {
            const auto& c = static_cast<const plan::Conditional&>(*op);
            if (!emitConditionalFields(h, module, c.thenMembers, error)) return false;
            if (!emitConditionalFields(h, module, c.elseMembers, error)) return false;
        } else {
            error = "generated conditional type backend supports fields, bits and nested conditionals: ";
            return false;
        }
    }
    return true;
}

bool emitConditionalDecode(std::ostringstream& c, const plan::Module& module,
                           const plan::Conditional& cond, const std::string& obj, std::string& error);
bool emitConditionalEncode(std::ostringstream& c, const plan::Module& module,
                           const plan::Conditional& cond, const std::string& obj, std::string& error);

bool emitConditionalDecodeMembers(std::ostringstream& c, const plan::Module& module,
                                  const std::vector<std::unique_ptr<plan::Op>>& members,
                                  const std::string& obj, std::string& error) {
    for (const auto& op : members) {
        if (!op) { error = "null operation in conditional"; return false; }
        if (op->kind == plan::OpKind::Field) {
            if (!emitDecodeField(c, module, static_cast<const plan::Field&>(*op), obj, error)) return false;
        } else if (op->kind == plan::OpKind::Bits) {
            if (!emitBitsDecode(c, module, static_cast<const plan::Bits&>(*op), obj, error)) return false;
        } else if (op->kind == plan::OpKind::Conditional) {
            if (!emitConditionalDecode(c, module, static_cast<const plan::Conditional&>(*op), obj, error)) return false;
        } else {
            error = "generated conditional codec supports fields, bits and nested conditionals";
            return false;
        }
    }
    return true;
}

bool emitConditionalEncodeMembers(std::ostringstream& c, const plan::Module& module,
                                  const std::vector<std::unique_ptr<plan::Op>>& members,
                                  const std::string& obj, std::string& error) {
    for (const auto& op : members) {
        if (!op) { error = "null operation in conditional"; return false; }
        if (op->kind == plan::OpKind::Field) {
            if (!emitEncodeField(c, module, static_cast<const plan::Field&>(*op), obj, error)) return false;
        } else if (op->kind == plan::OpKind::Bits) {
            if (!emitBitsEncode(c, module, static_cast<const plan::Bits&>(*op), obj, error)) return false;
        } else if (op->kind == plan::OpKind::Conditional) {
            if (!emitConditionalEncode(c, module, static_cast<const plan::Conditional&>(*op), obj, error)) return false;
        } else {
            error = "generated conditional codec supports fields, bits and nested conditionals";
            return false;
        }
    }
    return true;
}

bool emitConditionalDecode(std::ostringstream& c, const plan::Module& module,
                           const plan::Conditional& cond, const std::string& obj, std::string& error) {
    std::string expr = emitRequireExpr(module, cond.condition.get(), obj, error);
    if (expr.empty()) return false;
    c << "    if(" << expr << "){\n";
    if (!emitConditionalDecodeMembers(c, module, cond.thenMembers, obj, error)) return false;
    c << "    } else {\n";
    if (!emitConditionalDecodeMembers(c, module, cond.elseMembers, obj, error)) return false;
    c << "    }\n";
    return true;
}

bool emitConditionalEncode(std::ostringstream& c, const plan::Module& module,
                           const plan::Conditional& cond, const std::string& obj, std::string& error) {
    std::string expr = emitRequireExpr(module, cond.condition.get(), obj, error);
    if (expr.empty()) return false;
    c << "    if(" << expr << "){\n";
    if (!emitConditionalEncodeMembers(c, module, cond.thenMembers, obj, error)) return false;
    c << "    } else {\n";
    if (!emitConditionalEncodeMembers(c, module, cond.elseMembers, obj, error)) return false;
    c << "    }\n";
    return true;
}

bool emitStructType(std::ostringstream& h, const plan::Module& module, const plan::Struct& s, std::string& error) {
    const auto ns = splitNamespace(s.name);
    std::vector<std::string> parentNs(ns.begin(), ns.end() - (ns.empty() ? 0 : 1));
    for (const auto& op : s.members) {
        if (op && op->kind == plan::OpKind::Block) {
            if (!emitBlockType(h, module, s, static_cast<const plan::Block&>(*op), error)) return false;
        } else if (op && op->kind == plan::OpKind::Variant) {
            if (!emitVariantType(h, module, s, static_cast<const plan::Variant&>(*op), error)) return false;
        }
    }
    emitNamespaceOpen(h, parentNs);
    h << "struct " << leafName(s.name) << " {\n";
    if (!s.documentation.empty()) h << indent(1) << "/** " << s.documentation << " */\n";
    for (const auto& op : s.members) {
        if (!op) { error = "null operation in struct " + s.name; return false; }
        if (op->kind == plan::OpKind::Field) {
            const auto& f = static_cast<const plan::Field&>(*op);
            std::string ft = cppFieldType(module, f, error); if (ft.empty()) return false;
            h << indent(1) << ft << " " << leafName(f.name) << "{};\n";
        } else if (op->kind == plan::OpKind::Bits) {
            const auto& b = static_cast<const plan::Bits&>(*op);
            for (const auto& bf : b.fields) {
                std::string ft = bf.type.kind == core::TypeKind::Primitive ? cppTypeName(bf.type.name) :
                                 bf.type.kind == core::TypeKind::Named ? qualify(bf.type.name) : "";
                if (ft.empty()) { error = "unsupported generated bits field type: " + bf.name; return false; }
                h << indent(1) << ft << " " << leafName(bf.name) << "{};\n";
            }
        } else if (op->kind == plan::OpKind::Virtual) {
            const auto& v = static_cast<const plan::Virtual&>(*op);
            std::string vt = virtualCppType(v.expression ? v.expression->type : core::ExprType::Invalid, error); if (vt.empty()) return false;
            h << indent(1) << vt << " " << leafName(v.name) << "{};\n";
        } else if (op->kind == plan::OpKind::Alias) {
            const auto& a = static_cast<const plan::FieldAlias&>(*op);
            const auto* target = findFieldBySymbol(s, a.target);
            if (!target) { error = "generated alias target field is unavailable: " + a.targetName; return false; }
            h << indent(1) << "auto& " << leafName(a.name) << "() { return " << leafName(target->name) << "; }\n";
            h << indent(1) << "const auto& " << leafName(a.name) << "() const { return " << leafName(target->name) << "; }\n";
        } else if (op->kind == plan::OpKind::Conditional) {
            const auto& cnd = static_cast<const plan::Conditional&>(*op);
            if (!emitConditionalFields(h, module, cnd.thenMembers, error)) return false;
            if (!emitConditionalFields(h, module, cnd.elseMembers, error)) return false;
        } else if (op->kind == plan::OpKind::Variant) {
            const auto& v = static_cast<const plan::Variant&>(*op);
            h << indent(1) << variantTypeName(s, v) << " " << leafName(v.name) << "{};\n";
        } else if (op->kind == plan::OpKind::Block) {
            const auto& b = static_cast<const plan::Block&>(*op);
            h << indent(1) << leafName(s.name) << "_" << b.name << " " << leafName(b.name) << "{};\n";
        } else if (op->kind == plan::OpKind::At) {
            const auto& a = static_cast<const plan::At&>(*op);
            // at() changes the wire position but does not introduce a nested
            // C++ object; its declared members are fields of the containing
            // generated value.
            for (const auto& sub : a.members) {
                if (!sub) { error = "null operation in at() of " + s.name; return false; }
                if (sub->kind == plan::OpKind::Field) {
                    const auto& f = static_cast<const plan::Field&>(*sub);
                    std::string ft = cppFieldType(module, f, error);
                    if (ft.empty()) return false;
                    h << indent(1) << ft << " " << leafName(f.name) << "{};\n";
                } else if (sub->kind == plan::OpKind::Bits) {
                    const auto& bits = static_cast<const plan::Bits&>(*sub);
                    for (const auto& bf : bits.fields) {
                        std::string ft = bf.type.kind == core::TypeKind::Primitive ? cppTypeName(bf.type.name) :
                                         bf.type.kind == core::TypeKind::Named ? qualify(bf.type.name) : "";
                        if (ft.empty()) { error = "unsupported generated at() bits field type: " + bf.name; return false; }
                        h << indent(1) << ft << " " << leafName(bf.name) << "{};\n";
                    }
                } else if (sub->kind == plan::OpKind::Callback) {
                    // Callbacks are execution operations, not stored C++ fields.
                    continue;
                } else if (sub->kind == plan::OpKind::Align) {
                    continue;
                } else {
                    error = "generated at() type backend supports fields, bits, callbacks and alignment: " + s.name;
                    return false;
                }
            }
        } else if (op->kind == plan::OpKind::Callback) {
            // Callbacks are execution operations, not stored C++ fields.
            continue;
        } else if (op->kind == plan::OpKind::Align) {
            continue;
        } else {
            error = "generated type backend supports fields, blocks, callbacks, at() and alignment: " + s.name;
            return false;
        }
    }
    h << "};\n\n";
    emitNamespaceClose(h, parentNs); h << "\n";
    return true;
}
void emitStructForwardDecls(std::ostringstream& h, const plan::Module& module) {
    for (const auto& s : module.structs) {
        if (!s.name.empty()) {
            const auto ns = splitNamespace(s.name); std::vector<std::string> p(ns.begin(), ns.end() - (ns.empty()?0:1));
            emitNamespaceOpen(h,p); h << "struct " << leafName(s.name) << ";\n"; emitNamespaceClose(h,p); h << "\n";
        }
    }
}
bool emitEnum(std::ostringstream& h, const plan::Module& module, const plan::Enum& e, std::string& error) {
    std::string underlying; if (!resolveEnumUnderlying(module,e,underlying,error)) return false;
    const auto ns = splitNamespace(e.name); std::vector<std::string> p(ns.begin(),ns.end()-(ns.empty()?0:1));
    emitNamespaceOpen(h,p); h << "enum class " << leafName(e.name) << " : " << cppTypeName(underlying) << " {\n";
    for (std::size_t i=0;i<e.items.size();++i) { std::string lit=valueLiteral(e.items[i].value,cppTypeName(underlying),error); if(!error.empty()) return false; h<<indent(1)<<e.items[i].name<<" = "<<lit<<(i+1==e.items.size()?"\n":",\n"); }
    h << "};\n\n"; emitNamespaceClose(h,p); h<<"\n"; return true;
}
bool emitEnumForwardDecls(std::ostringstream& h, const plan::Module& module, std::string& error) {
    for (const auto& e : module.enums) {
        std::string underlying; if(!resolveEnumUnderlying(module,e,underlying,error)) return false;
        const auto ns=splitNamespace(e.name); std::vector<std::string> p(ns.begin(),ns.end()-(ns.empty()?0:1));
        emitNamespaceOpen(h,p); h<<"enum class "<<leafName(e.name)<<" : "<<cppTypeName(underlying)<<";\n"; emitNamespaceClose(h,p); h<<"\n";
    }
    return true;
}
bool emitAlias(std::ostringstream& h,const plan::Module& module,const plan::Alias& a,std::string& error){
    const auto ns=splitNamespace(a.name);std::vector<std::string> p(ns.begin(),ns.end()-(ns.empty()?0:1));
    emitNamespaceOpen(h,p); const plan::Type* base=resolveAlias(module,a.target,error); if(!base)return false;
    if(hasDynamicSuffix(*base)||!base->dimensions.empty()){error="array aliases are unsupported in generated type declarations: "+a.name;return false;}
    std::string target;
    if(a.target.kind==core::TypeKind::Named) target=qualify(a.target.name);
    else if(base->kind==core::TypeKind::Primitive) target=cppTypeName(base->name);
    else if(base->kind==core::TypeKind::Bytes) target="std::uint8_t";
    else if(base->kind==core::TypeKind::String) target="char";
    else if(base->kind==core::TypeKind::Named) target=qualify(base->name);
    else {error="unsupported alias target: "+a.name;return false;}
    h<<"using "<<leafName(a.name)<<" = "<<target<<";\n\n"; emitNamespaceClose(h,p);h<<"\n";return true;
}
bool emitConstant(std::ostringstream& h,const plan::Constant& c,std::string& error){
    const auto ns=splitNamespace(c.name);std::vector<std::string> p(ns.begin(),ns.end()-(ns.empty()?0:1));
    emitNamespaceOpen(h,p); const std::string t=std::holds_alternative<double>(c.value)?"double":std::holds_alternative<bool>(c.value)?"bool":std::holds_alternative<std::int64_t>(c.value)?"std::int64_t":"std::uint64_t";
    const std::string lit=valueLiteral(c.value,t,error); if(!error.empty())return false; h<<"inline constexpr "<<t<<" "<<leafName(c.name)<<" = "<<lit<<";\n\n";emitNamespaceClose(h,p);h<<"\n";return true;
}
std::string bodyDecodeName(const std::string& n) { return "decode_body_" + sanitizeIdentifier(n); }
std::string bodyEncodeName(const std::string& n) { return "encode_body_" + sanitizeIdentifier(n); }
bool emitBitsDecode(std::ostringstream& c, const plan::Module&, const plan::Bits& b, const std::string& obj, std::string& error) {
    if(b.storageBytes<1||b.storageBytes>8||b.totalBits<1||b.totalBits>64){error="invalid bits storage in C++ generator";return false;}
    const auto e=!b.fields.empty()?b.fields.front().endian:plan::Endian::Little;
    const std::string big=e==plan::Endian::Big?"true":e==plan::Endian::Native?"embx_generated_detail::hostLittle() ? false : true":"false";
    c<<"    const std::uint64_t bitsRaw=r.getUnsigned("<<b.storageBytes<<","<<big<<"); if(!r.ok) return false;\n";
    int cursor=0, cbits=static_cast<int>(b.storageBytes*8);
    for(const auto& bf:b.fields){
        if(bf.bits<1||bf.bits>64||cursor+bf.bits>b.totalBits){error="invalid bits field width in C++ generator";return false;}
        int shift=cbits-cursor-bf.bits; std::string mask=bf.bits==64?"UINT64_MAX":"((UINT64_C(1)<<" + std::to_string(bf.bits) + ")-1)";
        std::string type=bf.type.kind==core::TypeKind::Primitive?cppTypeName(bf.type.name):bf.type.kind==core::TypeKind::Named?qualify(bf.type.name):"";
        if(type.empty()){error="unsupported bits field type: "+bf.name;return false;}
        c<<"    "<<fieldExpr(obj,bf.name)<<"=static_cast<"<<type<<">((bitsRaw>>"<<shift<<")&"<<mask<<");\n"; cursor+=bf.bits;
    } return true;
}
bool emitBitsEncode(std::ostringstream& c, const plan::Module&, const plan::Bits& b, const std::string& obj, std::string& error) {
    if(b.storageBytes<1||b.storageBytes>8||b.totalBits<1||b.totalBits>64){error="invalid bits storage in C++ generator";return false;}
    const auto e=!b.fields.empty()?b.fields.front().endian:plan::Endian::Little; const std::string big=e==plan::Endian::Big?"true":e==plan::Endian::Native?"embx_generated_detail::hostLittle() ? false : true":"false"; c<<"    std::uint64_t bitsRaw=0;\n";
    int cursor=0, cbits=static_cast<int>(b.storageBytes*8);
    for(const auto& bf:b.fields){
        if(bf.bits<1||bf.bits>64||cursor+bf.bits>b.totalBits){error="invalid bits field width in C++ generator";return false;}
        int shift=cbits-cursor-bf.bits; std::string mask=bf.bits==64?"UINT64_MAX":"((UINT64_C(1)<<" + std::to_string(bf.bits) + ")-1)";
        c<<"    bitsRaw|=(static_cast<std::uint64_t>("<<fieldExpr(obj,bf.name)<<")&"<<mask<<")<<"<<shift<<";\n"; cursor+=bf.bits;
    } c<<"    w.putUnsigned(bitsRaw,"<<b.storageBytes<<","<<big<<");\n"; return true;
}
std::string arrayDimensionSizeExpr(const plan::Module& module, const core::Dimension& d,
                                   const std::string& object, std::string& error) {
    if (const auto extent = staticDimensionExtent(d)) return std::to_string(*extent) + "ULL";
    if (!d.expression) { error = "array dimension has no expression"; return {}; }
    std::string sx;
    if (!emitSizeExpr(module, d.expression.get(), object, sx, error)) return {};
    return sx;
}

bool emitDecodeLeaf(std::ostringstream& c, const plan::Module& module, const plan::Field& f,
                    const plan::Type& base, const std::string& dst, std::string& error) {
    if (base.kind == core::TypeKind::Bytes) { c << "    " << dst << "=r.d[r.p++];\n"; return true; }
    if (base.kind == core::TypeKind::String) { c << "    " << dst << "=static_cast<char>(r.d[r.p++]);\n"; return true; }
    if (base.kind == core::TypeKind::Primitive) { c << "    " << dst << "=static_cast<std::remove_reference_t<decltype(" << dst << ")>>(" << scalarDecode(f, base) << ");\n"; return true; }
    if (base.kind == core::TypeKind::Named) {
        if (findStruct(module, base)) { c << "    if(!" << bodyDecodeName(base.name) << "(r," << dst << generatedParameterCall(module) << ",error)) return false;\n"; return true; }
        if (const auto* e = findEnum(module, base)) { std::string u; if (!resolveEnumUnderlying(module, *e, u, error)) return false; c << "    " << dst << "=static_cast<" << qualify(base.name) << ">(" << scalarDecodeName(f, u) << ");\n"; return true; }
    }
    error = "unsupported generated field type: " + f.name; return false;
}

bool emitDecodeArrayLevel(std::ostringstream& c, const plan::Module& module, const plan::Field& f,
                          const plan::Type& base, const std::string& rootObj,
                          const std::string& lvalue, std::size_t dim, std::string& error) {
    if (dim == f.type.dimensions.size()) return emitDecodeLeaf(c, module, f, base, lvalue, error);
    const auto& d = f.type.dimensions[dim];
    if (d.kind == core::Dimension::Kind::Remaining) { error = "[*] only applies to bytes/string in generated codec: " + f.name; return false; }
    const auto extent = staticDimensionExtent(d);
    const std::string sizeExpr = arrayDimensionSizeExpr(module, d, rootObj, error);
    if (sizeExpr.empty()) return false;
    const std::string idx = "i" + std::to_string(dim);
    if (!extent) {
        c << "    std::uint64_t n" << dim << "64=" << sizeExpr << "; std::size_t n" << dim << "=0; if(!embx_generated_detail::checkedHostSize(n" << dim << "64,n" << dim << ")){if(error)*error=\"dynamic size exceeds host size_t\";return false;}\n";
        c << "    if(n" << dim << "64>1048576ULL){if(error)*error=\"array element limit exceeded\";return false;}\n";
        c << "    " << lvalue << ".clear(); " << lvalue << ".resize(n" << dim << ");\n";
        c << "    for(std::size_t " << idx << "=0;" << idx << "<n" << dim << ";++" << idx << "){\n";
    } else {
        c << "    for(std::size_t " << idx << "=0;" << idx << "<" << sizeExpr << ";++" << idx << "){\n";
    }
    if (!emitDecodeArrayLevel(c, module, f, base, rootObj, lvalue + "[" + idx + "]", dim + 1, error)) return false;
    c << "    }\n";
    return true;
}

bool emitDecodeField(std::ostringstream& c, const plan::Module& module, const plan::Field& f, const std::string& obj, std::string& error) {
    const plan::Type* base=nullptr; if(!resolveFieldType(module,f,base,error)) return false;
    const std::string dst=fieldExpr(obj,f.name);
    if (!f.type.terminator.empty()) {
        if (!f.type.maxPayload.has_value()) { error = "generated invalid terminated sequence: " + f.name; return false; }
        c << "    { const std::vector<std::uint8_t> term{";
        for (std::size_t i = 0; i < f.type.terminator.size(); ++i) { if (i) c << ","; c << static_cast<unsigned>(f.type.terminator[i]); }
        c << "};";
        if (f.type.dimensions.size() == 1 && f.type.dimensions[0].kind == core::Dimension::Kind::Remaining) {
            c << " const std::size_t saved=r.p; " << dst << ".clear(); while(true){ if(r.atTerm(term)){r.consumeTerm(term);break;} if(r.p-saved>=" << *f.type.maxPayload << "ULL){r.p=saved;r.ok=false;r.error=\"terminated sequence terminator not found within maximum payload length\";if(error)*error=r.error;return false;} " << dst << ".emplace_back();";
            if(!emitDecodeLeaf(c,module,f,*base,dst+".back()",error)) return false;
            c << " if(r.p-saved>" << *f.type.maxPayload << "ULL){r.p=saved;r.ok=false;r.error=\"terminated sequence payload exceeds maximum length\";if(error)*error=r.error;return false;} } }\n";
        } else {
            if (base->kind != core::TypeKind::Bytes) { error = "generated invalid terminated sequence: " + f.name; return false; }
            c << " if(!r.getTerminatedBytes(" << dst << ",term," << *f.type.maxPayload << "ULL,error)) return false; }\n";
        }
    } else if (f.type.dimensions.size()==1 && f.type.dimensions[0].kind==core::Dimension::Kind::Remaining) {
        if(base->kind==core::TypeKind::Bytes) c<<"    r.getBytes("<<dst<<",r.remaining());\n";
        else if(base->kind==core::TypeKind::String) c<<"    r.getString("<<dst<<",r.remaining());\n";
        else {error="[*] only applies to bytes/string in generated codec: "+f.name;return false;}
    } else if(!f.type.dimensions.empty()) {
        if((base->kind==core::TypeKind::Bytes||base->kind==core::TypeKind::String)&&f.type.dimensions.size()==1) {
            const std::string sx=arrayDimensionSizeExpr(module,f.type.dimensions[0],obj,error); if(sx.empty()) return false;
            c<<"    std::uint64_t n64="<<sx<<"; std::size_t n=0; if(!embx_generated_detail::checkedHostSize(n64,n)){if(error)*error=\"dynamic size exceeds host size_t\";return false;}\n";
            if(base->kind==core::TypeKind::Bytes)c<<"    r.getBytes("<<dst<<",n);\n"; else c<<"    r.getString("<<dst<<",n);\n";
        } else if(!emitDecodeArrayLevel(c,module,f,*base,obj,dst,0,error)) return false;
    } else if(base->kind==core::TypeKind::Primitive || base->kind==core::TypeKind::Named) {
        if(!emitDecodeLeaf(c,module,f,*base,dst,error)) return false;
    } else {error="unsupported generated field type: "+f.name;return false;}
    c<<"    if(!r.ok){if(error)*error=r.error;return false;}\n";
    return true;
}

bool emitEncodeLeaf(std::ostringstream& c, const plan::Module& module, const plan::Field& f,
                    const plan::Type& base, const std::string& src, std::string& error) {
    if (base.kind == core::TypeKind::Bytes || base.kind == core::TypeKind::String) { c << "    w.putUnsigned(static_cast<std::uint8_t>(" << src << "),1,true);\n"; return true; }
    if (base.kind == core::TypeKind::Primitive) { c << "    " << scalarEncode(f,base,src) << "\n"; return true; }
    if (base.kind == core::TypeKind::Named) {
        if (findStruct(module,base)) { c << "    if(!" << bodyEncodeName(base.name) << "(" << src << ",output" << generatedParameterCall(module) << ",error)) return false;\n"; return true; }
        if (const auto* e=findEnum(module,base)) { std::string u; if(!resolveEnumUnderlying(module,*e,u,error)) return false; c << "    " << scalarEncodeName(f,u,"static_cast<"+cppTypeName(u)+">("+src+")") << "\n"; return true; }
    }
    error="unsupported generated field type: "+f.name; return false;
}

bool emitEncodeArrayLevel(std::ostringstream& c, const plan::Module& module, const plan::Field& f,
                          const plan::Type& base, const std::string& rootObj,
                          const std::string& lvalue, std::size_t dim, std::string& error) {
    if (dim == f.type.dimensions.size()) return emitEncodeLeaf(c,module,f,base,lvalue,error);
    const auto& d=f.type.dimensions[dim];
    if (d.kind==core::Dimension::Kind::Remaining) {error="[*] only applies to bytes/string in generated codec: "+f.name;return false;}
    const auto extent=staticDimensionExtent(d);
    const std::string sizeExpr=arrayDimensionSizeExpr(module,d,rootObj,error); if(sizeExpr.empty()) return false;
    const std::string idx="i"+std::to_string(dim);
    if(!extent){
        c<<"    std::uint64_t n"<<dim<<"64="<<sizeExpr<<"; if(n"<<dim<<"64>1048576ULL){if(error)*error=\"array element limit exceeded\";return false;}\n";
        c<<"    if(static_cast<std::uint64_t>("<<lvalue<<".size())!=n"<<dim<<"64){if(error)*error=\"dynamic field size mismatch\";return false;}\n";
        c<<"    for(std::size_t "<<idx<<"=0;"<<idx<<"<"<<lvalue<<".size();++"<<idx<<"){\n";
    } else c<<"    for(std::size_t "<<idx<<"=0;"<<idx<<"<"<<sizeExpr<<";++"<<idx<<"){\n";
    if(!emitEncodeArrayLevel(c,module,f,base,rootObj,lvalue+"["+idx+"]",dim+1,error)) return false;
    c<<"    }\n";
    return true;
}

bool emitEncodeField(std::ostringstream& c, const plan::Module& module, const plan::Field& f, const std::string& obj, std::string& error) {
    const plan::Type* base=nullptr; if(!resolveFieldType(module,f,base,error)) return false;
    const std::string src=fieldExpr(obj,f.name);
    if (!f.type.terminator.empty()) {
        if (!f.type.maxPayload.has_value()) { error = "generated invalid terminated sequence: " + f.name; return false; }
        c << "    { const std::vector<std::uint8_t> term{";
        for (std::size_t i = 0; i < f.type.terminator.size(); ++i) { if (i) c << ","; c << static_cast<unsigned>(f.type.terminator[i]); }
        c << "};";
        if (f.type.dimensions.size() == 1 && f.type.dimensions[0].kind == core::Dimension::Kind::Remaining) {
            c << " const std::size_t saved=w.p; for(const auto& elem:" << src << "){";
            if(!emitEncodeLeaf(c,module,f,*base,"elem",error)) return false;
            c << " if(w.p-saved>" << *f.type.maxPayload << "ULL){if(error)*error=\"terminated sequence payload exceeds maximum length\";return false;} } if(w.p-saved>" << *f.type.maxPayload << "ULL){if(error)*error=\"terminated sequence payload exceeds maximum length\";return false;} w.putBytes(term); }\n";
        } else {
            if (base->kind != core::TypeKind::Bytes) { error = "generated invalid terminated sequence: " + f.name; return false; }
            c << " try { w.putTerminatedBytes(" << src << ",term," << *f.type.maxPayload << "ULL); } catch(const std::exception& ex){if(error)*error=ex.what();return false;} }\n";
        }
    } else if(f.type.dimensions.size()==1 && f.type.dimensions[0].kind==core::Dimension::Kind::Remaining) {
        if(base->kind==core::TypeKind::Bytes)c<<"    w.putBytes("<<src<<");\n";
        else if(base->kind==core::TypeKind::String)c<<"    w.putString("<<src<<");\n";
        else {error="[*] only applies to bytes/string in generated codec: "+f.name;return false;}
    } else if(!f.type.dimensions.empty()) {
        if((base->kind==core::TypeKind::Bytes||base->kind==core::TypeKind::String)&&f.type.dimensions.size()==1){
            const std::string sx=arrayDimensionSizeExpr(module,f.type.dimensions[0],obj,error); if(sx.empty()) return false;
            c<<"    std::uint64_t n64="<<sx<<"; if(static_cast<std::uint64_t>("<<src<<".size())!=n64){if(error)*error=\"dynamic field size mismatch\";return false;}\n";
            if(base->kind==core::TypeKind::Bytes)c<<"    w.putBytes("<<src<<");\n"; else c<<"    w.putString("<<src<<");\n";
        } else if(!emitEncodeArrayLevel(c,module,f,*base,obj,src,0,error)) return false;
    } else if(base->kind==core::TypeKind::Primitive || base->kind==core::TypeKind::Named) {
        if(!emitEncodeLeaf(c,module,f,*base,src,error)) return false;
    } else {error="unsupported generated field type: "+f.name;return false;}
    return true;
}
bool emitBlockCodec(std::ostringstream& c,const plan::Module& module,const plan::Struct& parent,const plan::Block& b,std::string& error) {
    const std::string fn="block_"+sanitizeIdentifier(parent.name)+"__"+sanitizeIdentifier(b.name);
    const std::string type=qualify(leafName(parent.name)+"_"+b.name);
    c<<"static bool decode_"<<fn<<"(embx_generated_detail::Reader& r, "<<type<<"& out"<<generatedParameterSignature(module)<<", const embx_generated_detail::Callbacks* callbacks, std::string* error) {\n";
    for(const auto& op:b.members){
        if(!op || (op->kind!=plan::OpKind::Field && op->kind!=plan::OpKind::Bits && op->kind!=plan::OpKind::Align)){error="generated block codec supports fields, bits and alignment: "+b.name;return false;}
        if(op->kind==plan::OpKind::Field){if(!emitDecodeField(c,module,static_cast<const plan::Field&>(*op),"out",error))return false;}
        else if(op->kind==plan::OpKind::Bits){if(!emitBitsDecode(c,module,static_cast<const plan::Bits&>(*op),"out",error))return false;}
        else {const auto& a=static_cast<const plan::Align&>(*op);std::string sx;if(!emitAlignExpr(module,a,"out",sx,error))return false;c<<"    std::uint64_t align64="<<sx<<"; std::size_t alignN=0; if(align64==0 || !embx_generated_detail::checkedHostSize(align64,alignN)){if(error)*error=\"invalid alignment\";return false;} std::size_t pad=(alignN-(r.p%alignN))%alignN; if(!r.need(pad)) return false; r.p+=pad;\n";}
    }
    c<<"    return r.ok;\n}\n\n";
    c<<"static bool encode_"<<fn<<"(const "<<type<<"& value,std::vector<std::uint8_t>& output"<<generatedParameterSignature(module)<<", const embx_generated_detail::Callbacks* callbacks, std::string* error) {\n    embx_generated_detail::Writer w(output);\n";
    for(const auto& op:b.members){
        if(!op || (op->kind!=plan::OpKind::Field && op->kind!=plan::OpKind::Bits && op->kind!=plan::OpKind::Align)){error="generated block codec supports fields, bits and alignment: "+b.name;return false;}
        if(op->kind==plan::OpKind::Field){if(!emitEncodeField(c,module,static_cast<const plan::Field&>(*op),"value",error))return false;}
        else if(op->kind==plan::OpKind::Bits){if(!emitBitsEncode(c,module,static_cast<const plan::Bits&>(*op),"value",error))return false;}
        else {const auto& a=static_cast<const plan::Align&>(*op);std::string sx;if(!emitAlignExpr(module,a,"value",sx,error))return false;c<<"    std::uint64_t align64="<<sx<<"; std::size_t alignN=0; if(align64==0 || !embx_generated_detail::checkedHostSize(align64,alignN)){if(error)*error=\"invalid alignment\";return false;} std::size_t pad=(alignN-(w.p%alignN))%alignN; if(!w.ensure(pad)){if(error)*error=\"output size overflow\";return false;} w.p+=pad;\n";}
    }
    c<<"    return true;\n}\n\n";
    return true;
}
bool emitStructCodec(std::ostringstream& h,std::ostringstream& c,const plan::Module& module,const plan::Struct& s,std::string& error) {
    const auto ns=splitNamespace(s.name);std::vector<std::string> p(ns.begin(),ns.end()-(ns.empty()?0:1));const std::string leaf=leafName(s.name);const std::string fn=sanitizeIdentifier(s.name);
    for(const auto& op:s.members) if(op && op->kind==plan::OpKind::Block) if(!emitBlockCodec(c,module,s,static_cast<const plan::Block&>(*op),error))return false;
    c<<"static bool "<<bodyDecodeName(s.name)<<"(embx_generated_detail::Reader& r, "<<qualify(s.name)<<"& out"<<generatedParameterSignature(module)<<", const embx_generated_detail::Callbacks* callbacks, std::string* error) {\n";
    for(const auto& op:s.members){
        if(!op){error="null operation in struct "+s.name;return false;}
        if(op->kind==plan::OpKind::Field){if(!emitDecodeField(c,module,static_cast<const plan::Field&>(*op),"out",error))return false;}
        else if(op->kind==plan::OpKind::Bits){if(!emitBitsDecode(c,module,static_cast<const plan::Bits&>(*op),"out",error))return false;}
        else if(op->kind==plan::OpKind::Virtual){const auto& v=static_cast<const plan::Virtual&>(*op);std::string ex=emitCodeExpr(module,s,v.expression.get(),"out",error);if(ex.empty())return false;c<<"    out."<<leafName(v.name)<<"=static_cast<decltype(out."<<leafName(v.name)<<")>("<<ex<<");\n";}
        else if(op->kind==plan::OpKind::Alias){continue;}
        else if(op->kind==plan::OpKind::Callback){
            const auto& cb=static_cast<const plan::Callback&>(*op);
            if(cb.direction==core::CallbackDirection::Decode){
                if(!emitCallbackDecode(c,module,s,cb,"out",error))return false;
            } else if(cb.direction!=core::CallbackDirection::Encode){
                error="callback direction is unspecified: "+cb.name;
                return false;
            }
        }
        else if(op->kind==plan::OpKind::Conditional){
            if(!emitConditionalDecode(c,module,static_cast<const plan::Conditional&>(*op),"out",error))return false;
        }
        else if(op->kind==plan::OpKind::Variant){
            if(!emitVariantDecode(c,module,static_cast<const plan::Variant&>(*op),"out",error))return false;
        }
        else if(op->kind==plan::OpKind::At){
            const auto& a=static_cast<const plan::At&>(*op); std::string sx;if(!emitOffsetExpr(module,a,"out",sx,error))return false;
            c<<"    const std::size_t savedP=r.p; std::uint64_t at64="<<sx<<"; std::size_t atN=0; if(!embx_generated_detail::checkedHostSize(at64,atN)||atN<r.begin||atN>r.limit){if(error)*error=\"at offset outside active input\";return false;} r.p=atN;\n";
            for(const auto& sub:a.members){if(!sub){error="null at operation";return false;} if(sub->kind==plan::OpKind::Field){if(!emitDecodeField(c,module,static_cast<const plan::Field&>(*sub),"out",error))return false;} else if(sub->kind==plan::OpKind::Bits){if(!emitBitsDecode(c,module,static_cast<const plan::Bits&>(*sub),"out",error))return false;} else if(sub->kind==plan::OpKind::Callback){const auto& cb=static_cast<const plan::Callback&>(*sub); if(cb.direction==core::CallbackDirection::Decode){if(!emitCallbackDecode(c,module,s,cb,"out",error))return false;} else if(cb.direction!=core::CallbackDirection::Encode){error="callback direction is unspecified: "+cb.name;return false;}} else if(sub->kind==plan::OpKind::Align){const auto& al=static_cast<const plan::Align&>(*sub);std::string ax;if(!emitAlignExpr(module,al,"out",ax,error))return false;c<<"    std::uint64_t align64="<<ax<<"; std::size_t alignN=0; if(align64==0||!embx_generated_detail::checkedHostSize(align64,alignN)){if(error)*error=\"invalid alignment\";return false;} std::size_t pad=(alignN-(r.p%alignN))%alignN; if(!r.need(pad)) return false; r.p+=pad;\n";} else {error="generated at() supports fields, bits and alignment: "+s.name;return false;}}
            c<<"    r.p=savedP;\n";
        }
        else if(op->kind==plan::OpKind::Block){const auto& b=static_cast<const plan::Block&>(*op);std::string sx;if(!emitBlockSizeExpr(module,b,"out",sx,error))return false;const std::string bf=leafName(s.name)+"_"+b.name;c<<"    std::uint64_t blockN64="<<sx<<"; std::size_t blockN=0; if(!embx_generated_detail::checkedHostSize(blockN64,blockN)||blockN>r.remaining()){if(error)*error=\"input truncated\";return false;} embx_generated_detail::Reader br(r.d,r.p,blockN); if(!decode_block_"<<sanitizeIdentifier(s.name)<<"__"<<sanitizeIdentifier(b.name)<<"(br,out."<<leafName(b.name)<<generatedParameterCall(module)<<",error)) return false; if(br.p!=br.limit){if(error)*error=\"block size mismatch\";return false;} r.p=br.limit;\n";}
        else if(op->kind==plan::OpKind::Align){const auto& a=static_cast<const plan::Align&>(*op);std::string sx;if(!emitAlignExpr(module,a,"out",sx,error))return false;c<<"    std::uint64_t align64="<<sx<<"; std::size_t alignN=0; if(align64==0||!embx_generated_detail::checkedHostSize(align64,alignN)){if(error)*error=\"invalid alignment\";return false;} std::size_t pad=(alignN-(r.p%alignN))%alignN; if(!r.need(pad)) return false; r.p+=pad;\n";}
        else {error="unsupported generated operation in struct "+s.name;return false;}
    }
    for (const auto& req : s.requirements) { std::string rx=emitRequireExpr(module, req.get(),"out",error); if(rx.empty()) return false; c<<"    if(!("<<rx<<")){if(error)*error=\"requires constraint failed\";return false;}\n"; }
    c<<"    return r.ok;\n}\n\n";
    c<<"static bool "<<bodyEncodeName(s.name)<<"(const "<<qualify(s.name)<<"& value,std::vector<std::uint8_t>& output"<<generatedParameterSignature(module)<<", const embx_generated_detail::Callbacks* callbacks, std::string* error) {\n    auto work = value;\n    embx_generated_detail::Writer w(output);\n";
    for (const auto& req : s.requirements) { std::string rx=emitRequireExpr(module, req.get(),"work",error); if(rx.empty()) return false; c<<"    if(!("<<rx<<")){if(error)*error=\"requires constraint failed\";return false;}\n"; }
    for(const auto& op:s.members){
        if(!op){error="null operation in struct "+s.name;return false;}
        if(op->kind==plan::OpKind::Field){if(!emitEncodeField(c,module,static_cast<const plan::Field&>(*op),"work",error))return false;}
        else if(op->kind==plan::OpKind::Bits){if(!emitBitsEncode(c,module,static_cast<const plan::Bits&>(*op),"work",error))return false;}
        else if(op->kind==plan::OpKind::Virtual){continue;}
        else if(op->kind==plan::OpKind::Alias){continue;}
        else if(op->kind==plan::OpKind::Callback){
            const auto& cb=static_cast<const plan::Callback&>(*op);
            if(cb.direction==core::CallbackDirection::Encode){
                if(!emitCallbackEncode(c,module,s,cb,"work",error))return false;
            } else if(cb.direction!=core::CallbackDirection::Decode){
                error="callback direction is unspecified: "+cb.name;
                return false;
            }
        }
        else if(op->kind==plan::OpKind::Conditional){
            if(!emitConditionalEncode(c,module,static_cast<const plan::Conditional&>(*op),"work",error))return false;
        }
        else if(op->kind==plan::OpKind::Variant){
            if(!emitVariantEncode(c,module,static_cast<const plan::Variant&>(*op),"work",error))return false;
        }
        else if(op->kind==plan::OpKind::At){
            const auto& a=static_cast<const plan::At&>(*op); std::string sx;if(!emitOffsetExpr(module,a,"work",sx,error))return false;
            c<<"    const std::size_t savedP=w.p; std::uint64_t at64="<<sx<<"; std::size_t atN=0; if(!embx_generated_detail::checkedHostSize(at64,atN)){if(error)*error=\"at offset exceeds host size\";return false;} if(!w.seek(atN)){if(error)*error=\"at offset outside output\";return false;}\n";
            for(const auto& sub:a.members){if(!sub){error="null at operation";return false;} if(sub->kind==plan::OpKind::Field){if(!emitEncodeField(c,module,static_cast<const plan::Field&>(*sub),"work",error))return false;} else if(sub->kind==plan::OpKind::Bits){if(!emitBitsEncode(c,module,static_cast<const plan::Bits&>(*sub),"work",error))return false;} else if(sub->kind==plan::OpKind::Callback){const auto& cb=static_cast<const plan::Callback&>(*sub); if(cb.direction==core::CallbackDirection::Encode){if(!emitCallbackEncode(c,module,s,cb,"work",error))return false;} else if(cb.direction!=core::CallbackDirection::Decode){error="callback direction is unspecified: "+cb.name;return false;}} else if(sub->kind==plan::OpKind::Align){const auto& al=static_cast<const plan::Align&>(*sub);std::string ax;if(!emitAlignExpr(module,al,"work",ax,error))return false;c<<"    std::uint64_t align64="<<ax<<"; std::size_t alignN=0; if(align64==0||!embx_generated_detail::checkedHostSize(align64,alignN)){if(error)*error=\"invalid alignment\";return false;} std::size_t pad=(alignN-(alignN==0?0:(w.p%alignN)))%alignN; if(!w.ensure(pad)){if(error)*error=\"output size overflow\";return false;} w.p+=pad;\n";} else {error="generated at() supports fields, bits and alignment: "+s.name;return false;}}
            c<<"    w.p=savedP;\n";
        }
        else if(op->kind==plan::OpKind::Block){const auto& b=static_cast<const plan::Block&>(*op);std::string sx;if(!emitBlockSizeExpr(module,b,"work",sx,error))return false;c<<"    std::uint64_t blockN64="<<sx<<"; if(blockN64>1048576ULL*1048576ULL){if(error)*error=\"block size exceeds generated codec budget\";return false;} std::vector<std::uint8_t> blockBuf; if(!encode_block_"<<sanitizeIdentifier(s.name)<<"__"<<sanitizeIdentifier(b.name)<<"(work."<<leafName(b.name)<<",blockBuf"<<generatedParameterCall(module)<<",error)) return false; if(blockN64!=blockBuf.size()){if(error)*error=\"block size mismatch\";return false;} output.insert(output.end(),blockBuf.begin(),blockBuf.end());\n";}
        else if(op->kind==plan::OpKind::Align){const auto& a=static_cast<const plan::Align&>(*op);std::string sx;if(!emitAlignExpr(module,a,"work",sx,error))return false;c<<"    std::uint64_t align64="<<sx<<"; std::size_t alignN=0; if(align64==0||!embx_generated_detail::checkedHostSize(align64,alignN)){if(error)*error=\"invalid alignment\";return false;} std::size_t pad=(alignN-(w.p%alignN))%alignN; if(!w.ensure(pad)){if(error)*error=\"output size overflow\";return false;} w.p+=pad;\n";}
        else {error="unsupported generated operation in struct "+s.name;return false;}
    }
    c<<"    return true;\n}\n\n";
    emitNamespaceOpen(h,p);h<<"bool decode__"<<fn<<"(const std::vector<std::uint8_t>& input,"<<leaf<<"& out,std::size_t& consumed"<<generatedParameterSignature(module)<<", const embx_generated_detail::Callbacks* callbacks=nullptr, std::string* error=nullptr);\n";h<<"bool encode__"<<fn<<"(const "<<leaf<<"& value,std::vector<std::uint8_t>& output"<<generatedParameterSignature(module)<<", const embx_generated_detail::Callbacks* callbacks=nullptr, std::string* error=nullptr);\n\n";emitNamespaceClose(h,p);h<<"\n";
    emitNamespaceOpen(c,p);c<<"bool decode__"<<fn<<"(const std::vector<std::uint8_t>& input,"<<leaf<<"& out,std::size_t& consumed"<<generatedParameterSignature(module)<<", const embx_generated_detail::Callbacks* callbacks, std::string* error){ embx_generated_detail::Reader r(input); if(!"<<bodyDecodeName(s.name)<<"(r,out"<<generatedParameterCall(module)<<",callbacks,error)){consumed=r.p; return false;} consumed=r.p; return true; }\n\n";
    c<<"bool encode__"<<fn<<"(const "<<leaf<<"& value,std::vector<std::uint8_t>& output"<<generatedParameterSignature(module)<<", const embx_generated_detail::Callbacks* callbacks, std::string* error){ std::vector<std::uint8_t> tmp; if(!"<<bodyEncodeName(s.name)<<"(value,tmp"<<generatedParameterCall(module)<<",callbacks,error)) return false; output.swap(tmp); return true; }\n\n";emitNamespaceClose(c,p);c<<"\n";
    return true;
}
} // namespace
bool generateCpp(const plan::Module& module, Output& output, std::string& error) {
    error.clear();std::ostringstream h,c;
    for (const auto& a : module.aliases) { if (a.symbol == core::InvalidSymbolId) { error = "alias has invalid SymbolId: " + a.name; return false; } }
    for (const auto& e : module.enums) { if (e.symbol == core::InvalidSymbolId) { error = "enum has invalid SymbolId: " + e.name; return false; } for (const auto& item : e.items) if (item.symbol == core::InvalidSymbolId) { error = "enum item has invalid SymbolId: " + e.name + "::" + item.name; return false; } }
    for (const auto& cst : module.constants) if (cst.symbol == core::InvalidSymbolId) { error = "constant has invalid SymbolId: " + cst.name; return false; }
    h<<"// Generated by EmbX C++ code generator.\n// C++17 type + generated fixed/dynamic nested-layout codec backend.\n\n#pragma once\n#include <algorithm>\n#include <array>\n#include <cstdint>\n#include <cstring>\n#include <functional>\n#include <limits>\n#include <stdexcept>\n#include <unordered_map>\n#include <utility>\n#include <string>\n#include <type_traits>\n#include <variant>\n#include <vector>\n\n";
    c<<"// Generated by EmbX C++ code generator.\n#include \"generated.hpp\"\n\n";
    emitCodecHelpers(h,module);
    emitStructForwardDecls(h,module);
    if(!emitEnumForwardDecls(h,module,error))return false;
    std::unordered_set<std::size_t> emittedAliases;
    std::function<bool(std::size_t,std::unordered_set<std::string>&)> emitAliasDfs=[&](std::size_t idx,std::unordered_set<std::string>& stack){
        if (emittedAliases.count(idx)) return true;
        const auto& a=module.aliases[idx];
        if(!stack.insert(a.name).second){error="cyclic alias during C++ generation: "+a.name;return false;}
        if(a.target.kind==core::TypeKind::Named && a.target.reference.valid()){auto it=module.aliasIndexBySymbol.find(a.target.reference.id);if(it!=module.aliasIndexBySymbol.end()){if(!emitAliasDfs(it->second,stack))return false;}}
        stack.erase(a.name);
        if(!emitAlias(h,module,a,error))return false;
        emittedAliases.insert(idx);return true;};
    for(std::size_t i=0;i<module.aliases.size();++i){std::unordered_set<std::string> stack;if(!emitAliasDfs(i,stack))return false;}
    for(const auto& e:module.enums)if(!emitEnum(h,module,e,error))return false;
    for(const auto& cst:module.constants)if(!emitConstant(h,cst,error))return false;
    std::vector<std::size_t> structOrder;std::unordered_set<core::SymbolId> visiting,done;
    for (std::size_t i = 0; i < module.structs.size(); ++i) {
        if (module.structs[i].symbol == core::InvalidSymbolId) { error = "struct has invalid SymbolId: " + module.structs[i].name; return false; }
        if (!topoVisitStruct(module, module.structs[i].symbol, visiting, done, structOrder, error)) return false;
    }
    for(const auto idx:structOrder)if(!emitStructType(h,module,module.structs[idx],error))return false;
    for(const auto idx:structOrder)if(!emitStructCodec(h,c,module,module.structs[idx],error))return false;
    output.header=h.str();output.source=c.str();return true;
}
} // namespace embx::codegen
