#include "Gguf.h"

#include <algorithm>
#include <cstring>
#include <limits>

namespace embx::gguf {
namespace {

class Reader {
public:
    Reader(const std::uint8_t* data, std::size_t size) : data_(data), size_(size) {}

    std::size_t position() const noexcept { return pos_; }
    bool readU32(std::uint32_t& v, std::string& e) noexcept { return read(v, 4, e); }
    bool readU64(std::uint64_t& v, std::string& e) noexcept { return read(v, 8, e); }
    bool readU8(std::uint8_t& v, std::string& e) noexcept { return read(v, 1, e); }

    bool readBytes(std::size_t n, const std::uint8_t*& p, std::string& e) noexcept {
        if (n > size_ - pos_) { e = "GGUF truncated input"; return false; }
        p = data_ + pos_; pos_ += n; return true;
    }

    bool skip(std::size_t n, std::string& e) noexcept {
        const std::uint8_t* ignored = nullptr;
        return readBytes(n, ignored, e);
    }

    bool readString(std::string& s, std::size_t maxLength, std::string& e) noexcept {
        std::uint64_t len = 0;
        if (!readU64(len, e)) return false;
        if (len > maxLength || len > static_cast<std::uint64_t>(size_ - pos_)) {
            e = "GGUF string length is invalid"; return false;
        }
        const auto* p = data_ + pos_;
        s.assign(reinterpret_cast<const char*>(p), static_cast<std::size_t>(len));
        pos_ += static_cast<std::size_t>(len);
        return true;
    }

private:
    template<class T>
    bool read(T& v, std::size_t bytes, std::string& e) noexcept {
        const std::uint8_t* p = nullptr;
        if (!readBytes(bytes, p, e)) return false;
        std::uint64_t value = 0;
        for (std::size_t i = 0; i < bytes; ++i) value |= static_cast<std::uint64_t>(p[i]) << (8u * i);
        v = static_cast<T>(value);
        return true;
    }

    const std::uint8_t* data_;
    std::size_t size_;
    std::size_t pos_ = 0;
};

bool validKey(const std::string& key) noexcept {
    if (key.empty() || key.size() > 65535) return false;
    std::size_t segmentStart = 0;
    for (std::size_t i = 0; i <= key.size(); ++i) {
        if (i != key.size() && key[i] != '.') continue;
        if (i == segmentStart) return false;
        for (std::size_t j = segmentStart; j < i; ++j) {
            const unsigned char c = static_cast<unsigned char>(key[j]);
            if (!(c >= 'a' && c <= 'z') && !(c >= '0' && c <= '9') && c != '_') return false;
        }
        if (key[segmentStart] < 'a' || key[segmentStart] > 'z') return false;
        segmentStart = i + 1;
    }
    return true;
}

bool metadataType(std::uint32_t raw, MetadataType& out) noexcept {
    if (raw > 12) return false;
    out = static_cast<MetadataType>(raw);
    return true;
}

bool skipValue(Reader& r, MetadataType type, std::string& e, unsigned depth = 0) noexcept {
    if (depth > 64) { e = "GGUF metadata nesting is too deep"; return false; }
    switch (type) {
    case MetadataType::UInt8: case MetadataType::Int8: case MetadataType::Bool: return r.skip(1, e);
    case MetadataType::UInt16: case MetadataType::Int16: return r.skip(2, e);
    case MetadataType::UInt32: case MetadataType::Int32: case MetadataType::Float32: return r.skip(4, e);
    case MetadataType::UInt64: case MetadataType::Int64: case MetadataType::Float64: return r.skip(8, e);
    case MetadataType::String: {
        std::string ignored;
        return r.readString(ignored, std::numeric_limits<std::uint64_t>::max(), e);
    }
    case MetadataType::Array: {
        std::uint32_t raw = 0; std::uint64_t count = 0;
        if (!r.readU32(raw, e) || !r.readU64(count, e)) return false;
        MetadataType elementType;
        if (!metadataType(raw, elementType)) { e = "GGUF metadata array type is invalid"; return false; }
        for (std::uint64_t i = 0; i < count; ++i) {
            if (!skipValue(r, elementType, e, depth + 1)) return false;
        }
        return true;
    }
    }
    e = "GGUF metadata type is invalid";
    return false;
}

bool tensorType(std::uint32_t raw, TensorType& out) noexcept {
    switch (raw) {
    case 0: case 1: case 2: case 3: case 6: case 7: case 8: case 9:
    case 10: case 11: case 12: case 13: case 14: case 15: case 16:
    case 17: case 18: case 19: case 20: case 21: case 22: case 23:
    case 24: case 25: case 26: case 27: case 28: case 29: case 30:
    case 34: case 35: case 39:
        out = static_cast<TensorType>(raw); return true;
    default: return false;
    }
}

bool checkedAdd(std::size_t a, std::size_t b, std::size_t& out) noexcept {
    if (b > std::numeric_limits<std::size_t>::max() - a) return false;
    out = a + b; return true;
}

bool fixedElementSize(TensorType type, std::uint64_t& size) noexcept {
    switch (type) {
    case TensorType::F32: case TensorType::I32: size = 4; return true;
    case TensorType::F16: case TensorType::I16: case TensorType::BF16: size = 2; return true;
    case TensorType::I8: size = 1; return true;
    case TensorType::I64: case TensorType::F64: size = 8; return true;
    default: return false;
    }
}

bool checkedProduct(const std::vector<std::uint64_t>& dims, std::uint64_t& out) noexcept {
    out = 1;
    for (const auto d : dims) {
        if (d != 0 && out > std::numeric_limits<std::uint64_t>::max() / d) return false;
        out *= d;
    }
    return true;
}

} // namespace

bool parse(const std::uint8_t* data, std::size_t size, Document& out, std::string& error) noexcept {
    out = {};
    error.clear();
    if (data == nullptr && size != 0) { error = "GGUF input pointer is null"; return false; }
    Reader r(data, size);

    const std::uint8_t* magic = nullptr;
    if (!r.readBytes(4, magic, error) || std::memcmp(magic, "GGUF", 4) != 0) {
        if (error.empty()) error = "GGUF magic is invalid";
        else error = "GGUF header is truncated";
        return false;
    }
    if (!r.readU32(out.header.version, error)) return false;
    if (out.header.version != 3) { error = "unsupported GGUF version"; return false; }
    if (!r.readU64(out.header.tensorCount, error) || !r.readU64(out.header.metadataCount, error)) return false;
    if (out.header.tensorCount > 100000000ULL || out.header.metadataCount > 100000000ULL) {
        error = "GGUF count is unreasonably large"; return false;
    }

    out.metadata.reserve(static_cast<std::size_t>(out.header.metadataCount));
    for (std::uint64_t i = 0; i < out.header.metadataCount; ++i) {
        MetadataEntry entry;
        if (!r.readString(entry.key, 65535, error) || !validKey(entry.key)) {
            error = "GGUF metadata key is invalid"; return false;
        }
        std::uint32_t rawType = 0;
        if (!r.readU32(rawType, error) || !metadataType(rawType, entry.type)) {
            error = "GGUF metadata value type is invalid"; return false;
        }
        entry.valueOffset = r.position();
        if (entry.key == "general.alignment") {
            if (entry.type != MetadataType::UInt32) {
                error = "GGUF alignment metadata must be uint32"; return false;
            }
            std::uint32_t alignment = 0;
            if (!r.readU32(alignment, error)) return false;
            if (alignment == 0 || (alignment & (alignment - 1)) != 0 || alignment % 8 != 0) {
                error = "GGUF alignment is invalid"; return false;
            }
            out.alignment = alignment;
        } else if (!skipValue(r, entry.type, error)) {
            return false;
        }
        entry.valueEnd = r.position();
        out.metadata.push_back(std::move(entry));
    }

    out.tensors.reserve(static_cast<std::size_t>(out.header.tensorCount));
    for (std::uint64_t i = 0; i < out.header.tensorCount; ++i) {
        TensorInfo info;
        if (!r.readString(info.name, 64, error)) return false;
        std::uint32_t rank = 0;
        if (!r.readU32(rank, error) || rank > 64) { error = "GGUF tensor rank is invalid"; return false; }
        info.dimensions.resize(rank);
        for (auto& d : info.dimensions) {
            if (!r.readU64(d, error)) return false;
        }
        std::uint32_t rawType = 0;
        if (!r.readU32(rawType, error) || !tensorType(rawType, info.type)) {
            error = "GGUF tensor type is invalid"; return false;
        }
        if (!r.readU64(info.offset, error)) return false;
        info.descriptorOffset = r.position();
        out.tensors.push_back(std::move(info));
    }

    const std::size_t headerEnd = r.position();
    const auto alignment = out.alignment;
    const auto remainder = headerEnd % alignment;
    const auto padding = remainder == 0 ? 0 : alignment - remainder;
    if (!checkedAdd(headerEnd, padding, out.tensorDataOffset) || out.tensorDataOffset > size) {
        error = "GGUF tensor-data region is truncated"; return false;
    }

    for (const auto& tensor : out.tensors) {
        if (tensor.offset % alignment != 0) { error = "GGUF tensor offset is not aligned"; return false; }
        if (tensor.offset > static_cast<std::uint64_t>(size - out.tensorDataOffset)) {
            error = "GGUF tensor offset is outside the file"; return false;
        }
    }
    return true;
}

bool makeArrayView(const Document& document, std::size_t tensorIndex,
                   const std::uint8_t* fileData, std::size_t fileSize,
                   runtime::ArrayView& out, std::string& error) noexcept {
    out = {};
    error.clear();
    if (tensorIndex >= document.tensors.size()) { error = "GGUF tensor index is out of bounds"; return false; }
    if (fileData == nullptr) { error = "GGUF file pointer is null"; return false; }
    if (document.tensorDataOffset > fileSize) { error = "GGUF tensor-data offset exceeds file size"; return false; }
    const auto& tensor = document.tensors[tensorIndex];
    std::uint64_t elementSize = 0;
    if (!fixedElementSize(tensor.type, elementSize)) {
        error = "GGUF tensor type is not a fixed-size scalar representation"; return false;
    }
    std::uint64_t count = 0;
    if (!checkedProduct(tensor.dimensions, count)) { error = "GGUF tensor element count overflow"; return false; }
    if (count != 0 && elementSize > std::numeric_limits<std::uint64_t>::max() / count) {
        error = "GGUF tensor byte size overflow"; return false;
    }
    const auto byteSize64 = count * elementSize;
    if (tensor.offset > static_cast<std::uint64_t>(fileSize - document.tensorDataOffset) ||
        byteSize64 > static_cast<std::uint64_t>(fileSize - document.tensorDataOffset - static_cast<std::size_t>(tensor.offset))) {
        error = "GGUF tensor data exceeds file bounds"; return false;
    }
    if (byteSize64 > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
        error = "GGUF tensor data exceeds host size_t"; return false;
    }
    out.descriptor.elementKind = runtime::ArrayElementKind::Primitive;
    out.descriptor.elementSize = elementSize;
    out.descriptor.elementCount = count;
    // GGML dimension 0 is the fastest-varying dimension. EmbX uses the
    // opposite source-level convention: the last dimension is fastest.
    // Reverse only at the external-format boundary; TensorInfo keeps the
    // original GGUF order for diagnostics and conformance checks.
    out.descriptor.dimensions.assign(tensor.dimensions.rbegin(), tensor.dimensions.rend());
    out.buffer.data = fileData + document.tensorDataOffset + static_cast<std::size_t>(tensor.offset);
    out.buffer.size = static_cast<std::size_t>(byteSize64);
    return out.valid(&error);
}

} // namespace embx::gguf
