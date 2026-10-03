#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace embx::runtime {

// Runtime description of a contiguous array buffer.  This is deliberately
// smaller than core::Type: it describes the facts needed to interpret storage
// without creating a second semantic type system.
enum class ArrayElementKind {
    Unknown,
    Primitive,
    Bytes,
    String,
    Named
};

struct ArrayDescriptor {
    ArrayElementKind elementKind = ArrayElementKind::Unknown;
    std::uint64_t elementTypeId = 0; // stable semantic id for named elements
    std::uint64_t elementSize = 0;   // bytes occupied by one encoded element
    std::uint64_t elementCount = 0;  // product of dimensions
    std::vector<std::uint64_t> dimensions;

    bool valid(std::string* error = nullptr) const noexcept {
        if (elementSize == 0) {
            if (error) *error = "array element size must be non-zero";
            return false;
        }
        std::uint64_t count = 1;
        for (const auto extent : dimensions) {
            if (extent != 0 && count > std::numeric_limits<std::uint64_t>::max() / extent) {
                if (error) *error = "array element count overflow";
                return false;
            }
            count *= extent;
        }
        if (count != elementCount) {
            if (error) *error = "array element count does not match dimensions";
            return false;
        }
        return true;
    }
};

struct ArrayBuffer {
    const std::uint8_t* data = nullptr;
    std::size_t size = 0;

    bool empty() const noexcept { return size == 0; }
};

// Non-owning view.  The owner of buffer storage must outlive the view.
struct ArrayView {
    ArrayDescriptor descriptor;
    ArrayBuffer buffer;

    bool elementAt(const std::vector<std::uint64_t>& indices,
                   const std::uint8_t*& out,
                   std::string& error) const noexcept {
        out = nullptr;
        if (!valid(&error)) return false;
        if (indices.size() != descriptor.dimensions.size()) {
            error = "array index rank does not match dimensions";
            return false;
        }

        std::uint64_t linear = 0;
        std::uint64_t stride = 1;
        for (std::size_t i = indices.size(); i-- > 0;) {
            const auto index = indices[i];
            const auto extent = descriptor.dimensions[i];
            if (index >= extent) {
                error = "array index is out of bounds";
                return false;
            }
            if (index != 0 && stride > std::numeric_limits<std::uint64_t>::max() / index) {
                error = "array index overflow";
                return false;
            }
            const auto term = index * stride;
            if (linear > std::numeric_limits<std::uint64_t>::max() - term) {
                error = "array index overflow";
                return false;
            }
            linear += term;
            if (extent != 0 && stride > std::numeric_limits<std::uint64_t>::max() / extent) {
                error = "array index overflow";
                return false;
            }
            stride *= extent;
        }

        if (linear >= descriptor.elementCount) {
            error = "array index exceeds element count";
            return false;
        }
        if (descriptor.elementSize != 0 &&
            linear > std::numeric_limits<std::uint64_t>::max() / descriptor.elementSize) {
            error = "array element offset overflow";
            return false;
        }
        const auto offset = linear * descriptor.elementSize;
        if (offset > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
            error = "array element offset exceeds host size_t";
            return false;
        }
        out = buffer.data + static_cast<std::size_t>(offset);
        return true;
    }

    bool valid(std::string* error = nullptr) const noexcept {
        if (!descriptor.valid(error)) return false;
        if (descriptor.elementCount == 0) {
            if (buffer.size != 0) {
                if (error) *error = "empty array must have an empty buffer";
                return false;
            }
            return true;
        }
        if (descriptor.elementSize >
            static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
            if (error) *error = "array element size exceeds host size_t";
            return false;
        }
        const auto elementSize = static_cast<std::size_t>(descriptor.elementSize);
        if (descriptor.elementCount >
            static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max() / elementSize)) {
            if (error) *error = "array buffer size exceeds host size_t";
            return false;
        }
        const auto required = static_cast<std::size_t>(descriptor.elementCount) * elementSize;
        if (buffer.data == nullptr) {
            if (error) *error = "non-empty array requires a data pointer";
            return false;
        }
        if (buffer.size != required) {
            if (error) *error = "array buffer size does not match descriptor";
            return false;
        }
        return true;
    }
};

inline bool checkedElementCount(const std::vector<std::uint64_t>& dimensions,
                                std::uint64_t& count,
                                std::string& error) noexcept {
    count = 1;
    for (const auto extent : dimensions) {
        if (extent != 0 && count > std::numeric_limits<std::uint64_t>::max() / extent) {
            error = "array element count overflow";
            return false;
        }
        count *= extent;
    }
    return true;
}

} // namespace embx::runtime
