#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace embx::value {

struct Value {
    using Object = std::unordered_map<std::string, Value>;
    using Array = std::vector<Value>;
    using Bytes = std::vector<uint8_t>;
    using Storage = std::variant<std::monostate, int64_t, uint64_t, double, bool,
                                 std::string, Bytes, Object, Array>;

    Storage data;
    Value() = default;
    template<class T> Value(T v) : data(std::move(v)) {}

    enum class Kind { Null, Signed, Unsigned, Floating, Boolean, String, Bytes, Object, Array };
    Kind kind() const noexcept;
    const char* typeName() const noexcept;
    bool isNumber() const noexcept;

    const Value* get(const std::string& key) const noexcept;
    Value* get(const std::string& key) noexcept;
    const Value& at(const std::string& key) const;
    Value& at(const std::string& key);

    std::string debugString() const;
};

bool equal(const Value& a, const Value& b) noexcept;

} // namespace embx::value
