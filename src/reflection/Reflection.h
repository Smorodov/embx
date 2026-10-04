#pragma once
#include "plan/Plan.h"
#include <cstddef>
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace embx::reflection {

enum class Kind { Field, Virtual, Alias, Bits, Variant, Conditional, Block, At, Align, Callback };
enum class LayoutClass { Exact, Bounded, Unbounded, Unknown };
struct Member {
    Kind kind;
    std::string name;
    std::string type;
    std::string discriminator;
    std::optional<runtime::LayoutSize> staticSize;
    std::optional<runtime::LayoutSize> staticOffset;
    LayoutClass layoutClass = LayoutClass::Unknown;
    runtime::LayoutSize minSize = 0;
    std::optional<runtime::LayoutSize> maxSize;
    int bits = 0;
    std::vector<std::string> children;
    std::vector<std::string> attributes;
    std::string documentation;
    core::CallbackDirection callbackDirection = core::CallbackDirection::Unspecified;
    std::vector<std::string> callbackArgs;
    std::string transform;
    std::optional<double> transformFactor;
    bool terminated = false;
    std::vector<std::uint8_t> terminator;
    std::optional<runtime::LayoutSize> maxPayload;
    core::SymbolId symbol = core::InvalidSymbolId;
    core::SymbolId targetSymbol = core::InvalidSymbolId;
    // Structural children are preserved for composite operations.
    std::vector<Member> nested;
};
struct Struct {
    std::vector<std::string> attributes;
    std::string documentation;
    std::string name;
    plan::Endian endian;
    std::optional<runtime::LayoutSize> staticSize;
    runtime::LayoutSize minSize = 0;
    std::optional<runtime::LayoutSize> maxSize;
    LayoutClass layoutClass = LayoutClass::Unknown;
    std::vector<std::string> requirements;
    std::vector<Member> members;
};
struct EnumItem { std::string name; std::string documentation; };
struct Parameter { std::string name; core::SymbolId symbol = core::InvalidSymbolId; std::string type; std::string documentation; };
struct AttributeDefinition { std::string name; core::AttributeType type = core::AttributeType::Marker; };
struct Module {
    const plan::Module* plan = nullptr;
    std::string documentation;
    std::vector<AttributeDefinition> attributeDefinitions;
    std::vector<std::string> structNames;
    std::vector<std::vector<std::string>> structAttributes;
    std::vector<std::string> aliasNames;
    std::vector<std::vector<std::string>> aliasAttributes;
    std::vector<std::string> enumNames;
    std::vector<std::vector<std::string>> enumAttributes;
    std::vector<std::string> constantNames;
    std::vector<std::vector<std::string>> constantAttributes;
    std::vector<std::string> callbackNames;
    std::vector<std::vector<std::string>> callbackAttributes;
    std::vector<std::vector<EnumItem>> enumItems;
    std::vector<core::CallbackDirection> callbackDirections;
    std::vector<Parameter> parameters;
};

Module inspect(const plan::Module& plan);
std::optional<Struct> inspectStruct(const plan::Module& plan, const std::string& name);
const char* kindName(Kind kind) noexcept;

} // namespace embx::reflection
