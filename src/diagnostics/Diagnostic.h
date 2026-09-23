#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace embx::diagnostics {

enum class Code {
    None,
    UnknownStruct,
    UnknownType,
    MissingField,
    InvalidValue,
    OutOfRange,
    InputTruncated,
    OutputConstraint,
    VariantMismatch,
    BlockConstraint,
    ExpressionError,
    Internal
};

struct Diagnostic {
    Code code = Code::None;
    std::string message;
    std::string path;
    size_t byteOffset = 0;
    size_t expected = 0;
    size_t actual = 0;
    bool hasSizeDetail = false;

    explicit operator bool() const noexcept { return code != Code::None; }
};

const char* codeName(Code code) noexcept;
Code classify(const std::string& message, bool encoding = false) noexcept;
Diagnostic make(Code code, std::string message, std::string path = {}, size_t byteOffset = 0,
                size_t expected = 0, size_t actual = 0, bool hasSizeDetail = false);
std::string format(const Diagnostic& d);

} // namespace embx::diagnostics
