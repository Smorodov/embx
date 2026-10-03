#include "runtime/Array.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

using embx::runtime::ArrayDescriptor;
using embx::runtime::ArrayElementKind;
using embx::runtime::ArrayView;

int main() {
    std::vector<std::uint64_t> shape{2, 3};
    std::uint64_t count = 0;
    std::string error;
    assert(embx::runtime::checkedElementCount(shape, count, error));
    assert(count == 6);

    // The element is a named struct.  The descriptor is independent of the
    // struct's internal fields and therefore works for arrays of structures.
    ArrayDescriptor packet;
    packet.elementKind = ArrayElementKind::Named;
    packet.elementTypeId = 42;
    packet.elementSize = 12;
    packet.elementCount = count;
    packet.dimensions = shape;
    assert(packet.valid(&error));

    std::array<std::uint8_t, 72> storage{};
    ArrayView view{packet, {storage.data(), storage.size()}};
    assert(view.valid(&error));

    // Shape and storage are separate facts.  A different shape with the same
    // element count remains representable without changing the buffer.
    view.descriptor.dimensions = {3, 2};
    assert(view.valid(&error));

    view.descriptor.elementCount = 5;
    assert(!view.valid(&error));
    assert(error == "array element count does not match dimensions");

    // Restore the descriptor and verify canonical row-major indexing: the
    // last dimension is the fastest varying dimension.
    view.descriptor.elementCount = 6;
    view.descriptor.dimensions = {2, 3};
    const std::array<std::uint8_t, 72> indexed = {
        0,1,2,3,4,5,6,7,8,9,10,11,
        12,13,14,15,16,17,18,19,20,21,22,23,
        24,25,26,27,28,29,30,31,32,33,34,35,
        36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,
        60,61,62,63,64,65,66,67,68,69,70,71
    };
    view.buffer = {indexed.data(), indexed.size()};
    const std::uint8_t* element = nullptr;
    assert(view.elementAt({0, 0}, element, error));
    assert(element == indexed.data());
    assert(view.elementAt({0, 2}, element, error));
    assert(element == indexed.data() + 24);
    assert(view.elementAt({1, 0}, element, error));
    assert(element == indexed.data() + 36);
    assert(!view.elementAt({2, 0}, element, error));
    assert(error == "array index is out of bounds");
    assert(!view.elementAt({0}, element, error));
    assert(error == "array index rank does not match dimensions");

    return 0;
}
