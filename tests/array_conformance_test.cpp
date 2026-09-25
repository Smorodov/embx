#include "runtime/Array.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

using embx::runtime::ArrayDescriptor;
using embx::runtime::ArrayElementKind;
using embx::runtime::ArrayView;

static void checkShape(const std::vector<std::uint64_t>& shape,
                       std::uint64_t elementSize) {
    std::uint64_t count = 0;
    std::string error;
    assert(embx::runtime::checkedElementCount(shape, count, error));

    ArrayDescriptor descriptor;
    descriptor.elementKind = ArrayElementKind::Primitive;
    descriptor.elementSize = elementSize;
    descriptor.elementCount = count;
    descriptor.dimensions = shape;

    const auto byteCount = static_cast<std::size_t>(count * elementSize);
    std::vector<std::uint8_t> storage(byteCount);
    for (std::size_t i = 0; i < storage.size(); ++i) {
        storage[i] = static_cast<std::uint8_t>(i & 0xffu);
    }

    ArrayView view{descriptor, {storage.data(), storage.size()}};
    assert(view.valid(&error));

    const std::uint8_t* element = nullptr;
    std::vector<std::uint64_t> first(shape.size(), 0);
    assert(view.elementAt(first, element, error));
    assert(element == storage.data());

    // Verify the final element using the canonical last-dimension-fastest
    // linearization rule.
    std::vector<std::uint64_t> last(shape.size());
    for (std::size_t i = 0; i < shape.size(); ++i) {
        last[i] = shape[i] - 1;
    }
    assert(view.elementAt(last, element, error));
    assert(element == storage.data() + (storage.size() - elementSize));
}

static void checkIndexing3D() {
    ArrayDescriptor descriptor;
    descriptor.elementKind = ArrayElementKind::Primitive;
    descriptor.elementSize = 2;
    descriptor.elementCount = 24;
    descriptor.dimensions = {2, 3, 4};

    std::array<std::uint8_t, 48> storage{};
    ArrayView view{descriptor, {storage.data(), storage.size()}};
    std::string error;
    const std::uint8_t* element = nullptr;

    // ((i * 3) + j) * 4 + k, multiplied by element size.
    assert(view.elementAt({0, 0, 0}, element, error));
    assert(element == storage.data() + 0);
    assert(view.elementAt({0, 0, 3}, element, error));
    assert(element == storage.data() + 6);
    assert(view.elementAt({0, 1, 0}, element, error));
    assert(element == storage.data() + 8);
    assert(view.elementAt({1, 0, 0}, element, error));
    assert(element == storage.data() + 24);
    assert(view.elementAt({1, 2, 3}, element, error));
    assert(element == storage.data() + 46);

    assert(!view.elementAt({1, 3, 0}, element, error));
    assert(error == "array index is out of bounds");
}

static void checkZeroAndMalformedBuffers() {
    std::string error;

    ArrayDescriptor empty;
    empty.elementKind = ArrayElementKind::Primitive;
    empty.elementSize = 4;
    empty.elementCount = 0;
    empty.dimensions = {0, 3};

    ArrayView emptyView{empty, {nullptr, 0}};
    assert(emptyView.valid(&error));
    const std::uint8_t* element = nullptr;
    assert(!emptyView.elementAt({0, 0}, element, error));
    assert(error == "array index is out of bounds");

    std::array<std::uint8_t, 23> shortBuffer{};
    ArrayDescriptor malformed = empty;
    malformed.dimensions = {2, 3};
    malformed.elementCount = 6;
    malformed.elementSize = 4;
    ArrayView shortView{malformed, {shortBuffer.data(), shortBuffer.size()}};
    assert(!shortView.valid(&error));
    assert(error == "array buffer size does not match descriptor");

    ArrayDescriptor inconsistent = malformed;
    inconsistent.elementCount = 5;
    assert(!inconsistent.valid(&error));
    assert(error == "array element count does not match dimensions");
}

int main() {
    // Fixed × fixed, with both axis orders exercised.
    checkShape({2, 3}, 4);
    checkShape({3, 2}, 4);

    // Three-dimensional storage.
    checkShape({2, 3, 4}, 1);

    // Dynamic dimensions are represented here by their resolved runtime shape.
    checkShape({4, 3}, 4);
    checkShape({2, 5}, 4);
    checkShape({4, 5}, 4);

    // A named struct is represented by exactly the same runtime boundary; its
    // internal field layout is outside ArrayView and supplied by Plan.
    ArrayDescriptor named;
    named.elementKind = ArrayElementKind::Named;
    named.elementTypeId = 101;
    named.elementSize = 6;
    named.elementCount = 24;
    named.dimensions = {2, 3, 4};
    std::array<std::uint8_t, 144> nestedStorage{};
    ArrayView namedView{named, {nestedStorage.data(), nestedStorage.size()}};
    std::string error;
    const std::uint8_t* element = nullptr;
    assert(namedView.valid(&error));
    assert(namedView.elementAt({1, 2, 3}, element, error));
    assert(element == nestedStorage.data() + 138);

    checkIndexing3D();
    checkZeroAndMalformedBuffers();

    // Shape multiplication is checked before storage construction.
    std::uint64_t count = 0;
    assert(!embx::runtime::checkedElementCount(
        {std::numeric_limits<std::uint64_t>::max(), 2}, count, error));
    assert(error == "array element count overflow");

    return 0;
}
