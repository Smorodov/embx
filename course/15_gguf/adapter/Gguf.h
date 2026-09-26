#pragma once

#include "runtime/Array.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace embx::gguf {

enum class MetadataType : std::uint32_t {
    UInt8 = 0, Int8 = 1, UInt16 = 2, Int16 = 3, UInt32 = 4,
    Int32 = 5, Float32 = 6, Bool = 7, String = 8, Array = 9,
    UInt64 = 10, Int64 = 11, Float64 = 12
};

enum class TensorType : std::uint32_t {
    F32 = 0, F16 = 1, Q4_0 = 2, Q4_1 = 3, Q5_0 = 6, Q5_1 = 7,
    Q8_0 = 8, Q8_1 = 9, Q2_K = 10, Q3_K = 11, Q4_K = 12,
    Q5_K = 13, Q6_K = 14, Q8_K = 15, IQ2_XXS = 16, IQ2_XS = 17,
    IQ3_XXS = 18, IQ1_S = 19, IQ4_NL = 20, IQ3_S = 21, IQ2_S = 22,
    IQ4_XS = 23, I8 = 24, I16 = 25, I32 = 26, I64 = 27, F64 = 28,
    IQ1_M = 29, BF16 = 30, TQ1_0 = 34, TQ2_0 = 35, MXFP4 = 39
};

struct MetadataEntry {
    std::string key;
    MetadataType type = MetadataType::UInt8;
    std::size_t valueOffset = 0;
    std::size_t valueEnd = 0;
};

struct TensorInfo {
    std::string name;
    std::vector<std::uint64_t> dimensions;
    TensorType type = TensorType::F32;
    std::uint64_t offset = 0; // relative to tensor-data region
    std::size_t descriptorOffset = 0;
};

struct Header {
    std::uint32_t version = 0;
    std::uint64_t tensorCount = 0;
    std::uint64_t metadataCount = 0;
};

struct Document {
    Header header;
    std::uint64_t alignment = 32;
    std::size_t tensorDataOffset = 0;
    std::vector<MetadataEntry> metadata;
    std::vector<TensorInfo> tensors;
};

bool parse(const std::uint8_t* data, std::size_t size, Document& out, std::string& error) noexcept;

inline bool parse(const std::vector<std::uint8_t>& data, Document& out, std::string& error) noexcept {
    return parse(data.data(), data.size(), out, error);
}

// Creates a universal EmbX array view only for GGML types with a fixed-size
// unquantized scalar representation. Quantized GGUF types remain format-specific.
bool makeArrayView(const Document& document,
                   std::size_t tensorIndex,
                   const std::uint8_t* fileData,
                   std::size_t fileSize,
                   runtime::ArrayView& out,
                   std::string& error) noexcept;

} // namespace embx::gguf
