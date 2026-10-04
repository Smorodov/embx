#pragma once
#include <cstdint>
#include "core/Expr.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <cstdint>

namespace embx::ast {

enum class Endian { Inherit, Little, Big, Native };
using ExprKind = core::ExprKind;
enum class ModifierKind { Endian, Length, BitWidth };
enum class CallbackDirection { Unspecified, Decode, Encode };

inline CallbackDirection callbackDirectionFromName(const std::string& name) noexcept {
    if (name.rfind("on_decode", 0) == 0) return CallbackDirection::Decode;
    if (name.rfind("on_encode", 0) == 0) return CallbackDirection::Encode;
    return CallbackDirection::Unspecified;
}

inline bool isDecodeCallback(CallbackDirection d) noexcept {
    return d == CallbackDirection::Decode;
}

inline bool isEncodeCallback(CallbackDirection d) noexcept {
    return d == CallbackDirection::Encode;
}

enum class AttributeType { Marker, Integer, Float, String };
struct Attribute {
    std::string name;
    std::string value;
    bool hasValue = false;
};

using Expr = core::Expr;

struct TypeSuffix {
    bool dynamic = false;
    std::unique_ptr<Expr> expr;
};

struct TypeRef {
    std::string name;
    std::vector<TypeSuffix> suffixes;
    std::vector<std::uint8_t> terminator;
    std::optional<std::uint64_t> maxPayload;
};

struct FieldModifier {
    ModifierKind kind = ModifierKind::Length;
    Endian endian = Endian::Inherit;
    std::unique_ptr<Expr> expr;
    int bits = 0;
};

struct Member { virtual ~Member() = default; std::vector<Attribute> attributes; std::string documentation; };

struct Virtual : Member {
    std::string name;
    std::unique_ptr<Expr> expression;
};

struct Alias : Member {
    std::string name;
    std::string target;
};

struct Transform {
    std::string name;
    std::vector<std::unique_ptr<Expr>> arguments;
};

struct Field : Member {
    std::string name;
    TypeRef type;
    std::vector<FieldModifier> modifiers;
    Endian endian = Endian::Inherit;
    int bits = 0;
    std::string assertion;
    std::unique_ptr<Transform> transform;
};

struct Bits : Member { std::vector<std::unique_ptr<Field>> fields; };

struct VariantCase {
    std::unique_ptr<Expr> tag;
    std::unique_ptr<TypeRef> type;
    std::vector<std::unique_ptr<Member>> members;
};

struct Conditional : Member {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Member>> thenMembers;
    std::vector<std::unique_ptr<Member>> elseMembers;
};

struct Variant : Member {
    std::string name;
    std::unique_ptr<Expr> discriminator;
    std::vector<VariantCase> cases;
    std::unique_ptr<TypeRef> defaultType;
    std::vector<std::unique_ptr<Member>> defaultMembers;
};

struct Block : Member {
    std::string name;
    std::unique_ptr<Expr> size;
    std::vector<std::unique_ptr<Member>> members;
};

struct At : Member {
    std::unique_ptr<Expr> offset;
    std::vector<std::unique_ptr<Member>> members;
};

struct Align : Member { std::unique_ptr<Expr> alignment; explicit Align(std::unique_ptr<Expr> e): alignment(std::move(e)) {} };

struct Callback : Member {
    std::string name;
    CallbackDirection direction = CallbackDirection::Unspecified;
    std::vector<std::unique_ptr<Expr>> args;
};

struct Struct {
    std::vector<Attribute> attributes;
    std::vector<std::unique_ptr<Expr>> requirements;
    std::string documentation;
    std::string name;
    Endian endian = Endian::Inherit;
    std::vector<std::unique_ptr<Member>> members;
};

struct EnumItem { std::string name; std::unique_ptr<Expr> value; std::string documentation; };
struct Enum {
    std::vector<Attribute> attributes;
    std::string documentation;
    std::string name;
    std::unique_ptr<TypeRef> underlying;
    std::vector<EnumItem> items;
};

struct Const { std::vector<Attribute> attributes; std::string documentation; std::string name; std::unique_ptr<Expr> expr; bool computed = false; };
struct CallbackDecl { std::vector<Attribute> attributes; std::string documentation; std::string name; CallbackDirection direction = CallbackDirection::Unspecified; std::optional<std::string> parameter; };
struct Parameter { std::vector<Attribute> attributes; std::string documentation; std::string name; TypeRef type; };
struct TypeAlias { std::vector<Attribute> attributes; std::string documentation; std::string name; TypeRef target; };

struct TopLevelRef {
    enum class Kind { Alias, Struct, Enum, Const, Callback, Parameter };
    Kind kind;
    size_t index = 0;
};

struct Module {
    std::vector<Attribute> attributes;
    std::string documentation;
    std::vector<std::pair<std::string, AttributeType>> attributeDecls;
    Endian endian = Endian::Inherit;
    // Canonical namespace of this module. Empty means the global namespace.
    std::string nameSpace;
    // Source declaration order. Category vectors below provide typed access.
    std::vector<TopLevelRef> order;
    std::vector<TypeAlias> aliases;
    std::vector<std::unique_ptr<Struct>> structs;
    std::vector<Enum> enums;
    std::vector<Const> constants;
    std::vector<CallbackDecl> callbacks;
    std::vector<Parameter> parameters;
};

} // namespace embx::ast
