#pragma once
#include "core/Semantic.h"
#include "core/Type.h"
#include "core/Symbol.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdint>

namespace embx::ir {

using Type = core::Type;
using TypeKind = core::TypeKind;

enum class MemberKind { Field, Virtual, Alias, Bits, Variant, Conditional, Block, At, Align, Callback };
struct Member { virtual ~Member()=default; MemberKind kind; std::vector<core::Attribute> attributes; std::string documentation; };

struct Virtual : Member {
    core::SymbolId symbol = core::InvalidSymbolId;
    std::string name;
    std::unique_ptr<core::Expr> expression;
    Virtual(){kind=MemberKind::Virtual;}
};
struct FieldAlias : Member {
    core::SymbolId symbol = core::InvalidSymbolId;
    std::string name;
    core::SymbolId target = core::InvalidSymbolId;
    std::string targetName;
    FieldAlias(){kind=MemberKind::Alias;}
};

struct Transform {
    std::string name;
    std::vector<std::unique_ptr<core::Expr>> arguments;
};

struct Field : Member {
    core::SymbolId symbol = core::InvalidSymbolId;
    std::string name;
    Type type;
    int bits=0;
    core::Endian endian=core::Endian::Inherit;
    
    std::string assertion;
    std::unique_ptr<Transform> transform;
    Field(){kind=MemberKind::Field;}
};

struct Bits : Member {
    std::vector<std::unique_ptr<Field>> fields;
    int totalBits=0;
    Bits(){kind=MemberKind::Bits;}
};

struct VariantCase {
    std::unique_ptr<core::Expr> tag;
    Type type;
    bool hasType=false;
    std::vector<std::unique_ptr<Member>> members;
};
struct Conditional : Member {
    std::unique_ptr<core::Expr> condition;
    std::vector<std::unique_ptr<Member>> thenMembers;
    std::vector<std::unique_ptr<Member>> elseMembers;
    Conditional(){kind=MemberKind::Conditional;}
};
struct Variant : Member {
    std::string name;
    std::unique_ptr<core::Expr> discriminator;
    std::vector<VariantCase> cases;
    bool hasDefault=false;
    Type defaultType;
    std::vector<std::unique_ptr<Member>> defaultMembers;
    Variant(){kind=MemberKind::Variant;}
};
struct Block : Member {
    std::string name;
    std::unique_ptr<core::Expr> size;
    std::vector<std::unique_ptr<Member>> members;
    Block(){kind=MemberKind::Block;}
};
struct At : Member {
    std::unique_ptr<core::Expr> offset;
    std::vector<std::unique_ptr<Member>> members;
    At(){kind=MemberKind::At;}
};
struct Align : Member {
    std::unique_ptr<core::Expr> alignment;
    Align(){kind=MemberKind::Align;}
};
struct Callback : Member {
    std::string name;
    core::CallbackDirection direction = core::CallbackDirection::Unspecified;
    std::vector<std::unique_ptr<core::Expr>> args;
    Callback(){kind=MemberKind::Callback;}
};

struct Struct {
    core::SymbolId symbol = core::InvalidSymbolId;
    std::vector<core::Attribute> attributes;
    std::string documentation;
    std::string name;
    core::Endian endian=core::Endian::Inherit;
    std::vector<std::unique_ptr<core::Expr>> requirements;
    std::vector<std::unique_ptr<Member>> members;
};
struct EnumItem { core::SymbolId symbol = core::InvalidSymbolId; std::string name; std::unique_ptr<core::Expr> value; std::string documentation; };
struct Enum {
    core::SymbolId symbol = core::InvalidSymbolId;
    std::vector<core::Attribute> attributes;
    std::string documentation;
    std::string name;
    Type underlying;
    bool hasUnderlying=false;
    std::vector<EnumItem> items;
};
struct Alias { core::SymbolId symbol = core::InvalidSymbolId; std::vector<core::Attribute> attributes; std::string documentation; std::string name; Type target; };
struct Constant { core::SymbolId symbol = core::InvalidSymbolId; std::vector<core::Attribute> attributes; std::string documentation; std::string name; std::unique_ptr<core::Expr> expr; bool computed=false; };
struct CallbackDecl { core::SymbolId symbol = core::InvalidSymbolId; std::vector<core::Attribute> attributes; std::string documentation; std::string name; core::CallbackDirection direction=core::CallbackDirection::Unspecified; std::string parameter; bool hasParameter=false; };
struct Parameter { core::SymbolId symbol = core::InvalidSymbolId; std::vector<core::Attribute> attributes; std::string documentation; std::string name; Type type; };

struct TopLevelRef {
    enum class Kind { Alias, Struct, Enum, Const, Callback, Parameter };
    Kind kind;
    size_t index=0;
};

struct Module {
    std::string documentation;
    std::vector<core::AttributeDefinition> attributeDefinitions;
    // Canonical namespace retained in IR; declaration names are already fully qualified.
    std::string nameSpace;
    core::Endian endian=core::Endian::Inherit;
    std::vector<TopLevelRef> order;
    std::vector<Alias> aliases;
    std::vector<Enum> enums;
    std::vector<Constant> constants;
    std::vector<CallbackDecl> callbacks;
    std::vector<Parameter> parameters;
    std::vector<std::unique_ptr<Struct>> structs;
    core::SymbolTable symbolTable;
};

} // namespace embx::ir
