#pragma once
#include "core/Semantic.h"
#include "core/Type.h"
#include "core/Expr.h"
#include "runtime/ExpressionEvaluator.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace embx::ir { struct Module; }

namespace embx::plan {

using Expr = core::Expr;

using Type = core::Type;

enum class Endian { Little, Big, Native };

enum class OpKind { Field, Virtual, Alias, Bits, Variant, Conditional, Block, At, Align, Callback };
struct Op { virtual ~Op() = default; OpKind kind; std::vector<core::Attribute> attributes; std::string documentation; };

struct Virtual : Op {
    core::SymbolId symbol = core::InvalidSymbolId;
    std::string name;
    std::unique_ptr<Expr> expression;
    Virtual() { kind = OpKind::Virtual; }
};
struct FieldAlias : Op {
    core::SymbolId symbol = core::InvalidSymbolId;
    std::string name;
    core::SymbolId target = core::InvalidSymbolId;
    std::string targetName;
    FieldAlias() { kind = OpKind::Alias; }
};
struct Transform {
    std::string name;
    double factor = 1.0;
};

struct Field : Op {
    core::SymbolId symbol = core::InvalidSymbolId;
    std::string name;
    Type type;
    Endian endian = Endian::Little;
    std::optional<runtime::LayoutSize> staticLength;
    std::optional<runtime::LayoutSize> staticOffset;
    std::optional<runtime::LayoutSize> staticSize;
    int bits = 0;
    std::string assertion;
    std::optional<Transform> transform;
    Field() { kind = OpKind::Field; }
};
struct BitsField {
    core::SymbolId symbol = core::InvalidSymbolId;
    std::vector<core::Attribute> attributes;
    std::string name;
    std::string documentation;
    Type type;
    Endian endian = Endian::Little;
    int bits = 0;
    std::string assertion;
};
struct Bits : Op {
    std::vector<BitsField> fields;
    int totalBits = 0;
    runtime::LayoutSize storageBytes = 0;
    Bits() { kind = OpKind::Bits; }
};
struct VariantCase {
    runtime::Value tag;
    std::optional<Type> type;
    std::vector<std::unique_ptr<Op>> members;
};
struct Conditional : Op {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Op>> thenMembers;
    std::vector<std::unique_ptr<Op>> elseMembers;
    Conditional() { kind = OpKind::Conditional; }
};
struct Variant : Op {
    std::string name;
    std::unique_ptr<Expr> discriminator;
    std::vector<VariantCase> cases;
    bool hasDefault = false;
    std::optional<Type> defaultType;
    std::vector<std::unique_ptr<Op>> defaultMembers;
    Variant() { kind = OpKind::Variant; }
};
struct Block : Op {
    std::string name;
    std::unique_ptr<Expr> size;
    std::optional<runtime::LayoutSize> staticSize;
    std::vector<std::unique_ptr<Op>> members;
    Block() { kind = OpKind::Block; }
};
struct At : Op {
    std::unique_ptr<Expr> offset;
    std::optional<runtime::LayoutSize> staticOffset;
    std::vector<std::unique_ptr<Op>> members;
    At() { kind = OpKind::At; }
};
struct Align : Op {
    std::unique_ptr<Expr> alignment;
    std::optional<runtime::LayoutSize> staticAlignment;
    Align() { kind = OpKind::Align; }
};
struct Callback : Op {
    std::string name;
    core::CallbackDirection direction = core::CallbackDirection::Unspecified;
    std::vector<std::unique_ptr<Expr>> args;
    Callback() { kind = OpKind::Callback; }
};

struct Struct {
    core::SymbolId symbol = core::InvalidSymbolId;
    std::vector<core::Attribute> attributes;
    std::string documentation;
    std::string name;
    Endian endian = Endian::Little;
    std::vector<std::unique_ptr<Expr>> requirements;
    std::vector<std::unique_ptr<Op>> members;
    std::optional<runtime::LayoutSize> staticSize;
    runtime::LayoutSize minSizeInBytes = 0;
    std::optional<runtime::LayoutSize> sizeInBytes;
    std::optional<runtime::LayoutSize> maxSizeInBytes;
};
struct Constant { core::SymbolId symbol = core::InvalidSymbolId; std::vector<core::Attribute> attributes; std::string documentation; std::string name; runtime::Value value; bool computed = false; };
struct Alias { core::SymbolId symbol = core::InvalidSymbolId; std::vector<core::Attribute> attributes; std::string documentation; std::string name; Type target; };
struct EnumItem { core::SymbolId symbol = core::InvalidSymbolId; std::string name; runtime::Value value; std::string documentation; };
struct Enum { core::SymbolId symbol = core::InvalidSymbolId; std::vector<core::Attribute> attributes; std::string documentation; std::string name; Type underlying; std::unordered_map<std::string, runtime::Value> values; std::vector<EnumItem> items; };
struct CallbackDecl { core::SymbolId symbol = core::InvalidSymbolId; std::vector<core::Attribute> attributes; std::string documentation; std::string name; core::CallbackDirection direction = core::CallbackDirection::Unspecified; bool hasParameter = false; std::string parameter; };
struct Parameter { core::SymbolId symbol = core::InvalidSymbolId; std::vector<core::Attribute> attributes; std::string documentation; std::string name; Type type; };

enum class LayoutClass {
    Exact,
    Bounded,
    Unbounded
};

struct LayoutBounds {
    runtime::LayoutSize minSize = 0;
    std::optional<runtime::LayoutSize> maxSize;

    LayoutClass classification() const noexcept {
        if (!maxSize) return LayoutClass::Unbounded;
        return *maxSize == minSize ? LayoutClass::Exact : LayoutClass::Bounded;
    }

    bool exact() const noexcept { return maxSize && *maxSize == minSize; }
    bool bounded() const noexcept { return maxSize.has_value(); }
};

struct Module {
    std::string documentation;
    std::vector<core::AttributeDefinition> attributeDefinitions;
    Endian endian = Endian::Little;
    std::vector<Alias> aliases;
    std::vector<Struct> structs;
    std::vector<Enum> enums;
    std::vector<Constant> constants;
    std::vector<CallbackDecl> callbacks;
    std::vector<Parameter> parameters;
    std::unordered_map<core::SymbolId, std::size_t> parameterIndexBySymbol;
    core::SymbolTable symbolTable;
    std::unordered_map<core::SymbolId, std::size_t> constantIndexBySymbol;
    std::unordered_map<core::SymbolId, std::size_t> structIndexBySymbol;
    std::unordered_map<core::SymbolId, std::size_t> aliasIndexBySymbol;
    std::unordered_map<core::SymbolId, std::size_t> enumIndexBySymbol;
};

std::unique_ptr<Module> build(const ir::Module& ir, std::string& error);
std::optional<runtime::LayoutSize> fixedTypeSize(const Module& plan, const Type& type, std::string& error);
LayoutBounds layoutBounds(const Module& plan, const Type& type, std::string& error);

// Generated size properties are derived exclusively from LayoutBounds.
std::optional<runtime::LayoutSize> sizeInBytes(const Module& plan, const Type& type, std::string& error);
std::optional<runtime::LayoutSize> minSizeInBytes(const Module& plan, const Type& type, std::string& error);
std::optional<runtime::LayoutSize> maxSizeInBytes(const Module& plan, const Type& type, std::string& error);

} // namespace embx::plan
