#pragma once
#include <string>

namespace embx::core {

enum class Endian { Inherit, Little, Big, Native };
enum class CallbackDirection { Unspecified, Decode, Encode };
enum class AttributeType { Marker, Integer, Float, String };

struct Attribute {
    std::string name;
    std::string value;
    bool hasValue = false;
};

struct AttributeDefinition {
    std::string name;
    AttributeType type = AttributeType::Marker;
};

inline CallbackDirection callbackDirectionFromName(const std::string& name) noexcept {
    if (name.rfind("on_decode", 0) == 0) return CallbackDirection::Decode;
    if (name.rfind("on_encode", 0) == 0) return CallbackDirection::Encode;
    return CallbackDirection::Unspecified;
}
inline bool isDecodeCallback(CallbackDirection d) noexcept { return d == CallbackDirection::Decode; }
inline bool isEncodeCallback(CallbackDirection d) noexcept { return d == CallbackDirection::Encode; }

} // namespace embx::core
