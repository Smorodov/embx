#include "Gguf.h"
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace {
void u32(std::vector<std::uint8_t>& b, std::uint32_t v) { for (int i=0;i<4;++i) b.push_back(static_cast<std::uint8_t>(v>>(8*i))); }
void u64(std::vector<std::uint8_t>& b, std::uint64_t v) { for (int i=0;i<8;++i) b.push_back(static_cast<std::uint8_t>(v>>(8*i))); }
void str(std::vector<std::uint8_t>& b, const std::string& s) { u64(b,s.size()); b.insert(b.end(),s.begin(),s.end()); }
void align32(std::vector<std::uint8_t>& b) { while (b.size()%32) b.push_back(0); }
std::vector<std::uint8_t> fixture(bool badMagic=false, bool badOffset=false) {
    std::vector<std::uint8_t> b{'G','G','U','F'};
    u32(b,3); u64(b,1); u64(b,1);
    str(b,"general.alignment"); u32(b,4); u32(b,32);
    str(b,"tensor"); u32(b,2); u64(b,2); u64(b,3); u32(b,0); u64(b,badOffset?64:0);
    align32(b);
    b.insert(b.end(), {1,0,0,0, 2,0,0,0, 3,0,0,0, 4,0,0,0, 5,0,0,0, 6,0,0,0});
    if (badMagic) b[0]='X';
    return b;
}
}

TEST_CASE("GGUF adapter parses structure without changing EmbX core") {
    auto bytes = fixture();
    embx::gguf::Document doc; std::string error;
    REQUIRE(embx::gguf::parse(bytes, doc, error));
    REQUIRE(doc.header.version == 3);
    REQUIRE(doc.header.tensorCount == 1);
    REQUIRE(doc.metadata.size() == 1);
    REQUIRE(doc.alignment == 32);
    REQUIRE(doc.tensors[0].dimensions == std::vector<std::uint64_t>{2,3});
    REQUIRE(doc.tensors[0].type == embx::gguf::TensorType::F32);
    REQUIRE(doc.tensorDataOffset == 128);

    embx::runtime::ArrayView view; REQUIRE(embx::gguf::makeArrayView(doc,0,bytes.data(),bytes.size(),view,error));
    // GGML stores dimension 0 as fastest-varying; the adapter reverses the
    // external shape so EmbX keeps its canonical last-dimension-fastest order.
    REQUIRE(view.descriptor.dimensions == std::vector<std::uint64_t>{3,2});
    REQUIRE(view.descriptor.elementSize == 4);
    REQUIRE(view.descriptor.elementCount == 6);
    const std::uint8_t* p = nullptr;
    REQUIRE(view.elementAt({2,1},p,error));
    REQUIRE(p == view.buffer.data + 20);
}

TEST_CASE("GGUF adapter rejects malformed structural input") {
    embx::gguf::Document doc; std::string error;
    auto badMagic = fixture(true); REQUIRE_FALSE(embx::gguf::parse(badMagic,doc,error));
    auto badOffset = fixture(false,true); REQUIRE_FALSE(embx::gguf::parse(badOffset,doc,error));
    auto unaligned = fixture();
    unaligned[95] = 1; unaligned[96] = unaligned[97] = unaligned[98] = unaligned[99] =
        unaligned[100] = unaligned[101] = unaligned[102] = 0;
    REQUIRE_FALSE(embx::gguf::parse(unaligned,doc,error));
    auto badAlignmentType = fixture();
    // general.alignment value type at byte 51: change uint32 -> string.
    badAlignmentType[49] = 8; badAlignmentType[50] = badAlignmentType[51] = badAlignmentType[52] = 0;
    REQUIRE_FALSE(embx::gguf::parse(badAlignmentType,doc,error));
    auto truncated = fixture(); truncated.resize(20); REQUIRE_FALSE(embx::gguf::parse(truncated,doc,error));
}

TEST_CASE("GGUF adapter keeps quantized representation outside universal array semantics") {
    auto bytes = fixture();
    // Change tensor type from F32 to Q4_0 while keeping the structural fixture valid.
    // Header: 32 bytes; metadata: 32+? locate the type by the parser itself via a known fixture offset.
    // Rebuild a tiny equivalent by changing the first occurrence of the F32 tag after tensor dimensions.
    const std::size_t typeOffset = 91; // immediately after name, rank, and two dimensions
    REQUIRE(typeOffset + 4 <= bytes.size());
    bytes[typeOffset] = 2; bytes[typeOffset+1]=bytes[typeOffset+2]=bytes[typeOffset+3]=0;
    embx::gguf::Document doc; std::string error;
    REQUIRE(embx::gguf::parse(bytes,doc,error));
    embx::runtime::ArrayView view;
    REQUIRE_FALSE(embx::gguf::makeArrayView(doc,0,bytes.data(),bytes.size(),view,error));
}


TEST_CASE("GGUF adapter maps a non-square 3-D tensor at the byte boundary") {
    std::vector<std::uint8_t> b{'G','G','U','F'};
    u32(b,3); u64(b,1); u64(b,0);
    str(b,"tensor3d"); u32(b,3); u64(b,2); u64(b,3); u64(b,4); u32(b,0); u64(b,0);
    align32(b);
    for (std::uint32_t i=0; i<24; ++i) u32(b, i + 1);

    embx::gguf::Document doc; std::string error;
    REQUIRE(embx::gguf::parse(b, doc, error));
    REQUIRE(doc.tensorDataOffset == 96);
    REQUIRE(doc.tensors[0].dimensions == std::vector<std::uint64_t>{2,3,4});

    embx::runtime::ArrayView view;
    REQUIRE(embx::gguf::makeArrayView(doc, 0, b.data(), b.size(), view, error));
    REQUIRE(view.descriptor.dimensions == std::vector<std::uint64_t>{4,3,2});
    REQUIRE(view.descriptor.elementCount == 24);
    const std::uint8_t* p = nullptr;
    REQUIRE(view.elementAt({3,2,1}, p, error));
    REQUIRE(p == view.buffer.data + 23 * 4);
}

TEST_CASE("GGUF adapter handles multiple tensors with aligned data regions") {
    std::vector<std::uint8_t> b{'G','G','U','F'};
    u32(b,3); u64(b,2); u64(b,0);
    str(b,"a"); u32(b,1); u64(b,2); u32(b,0); u64(b,0);
    str(b,"b"); u32(b,2); u64(b,3); u64(b,2); u32(b,0); u64(b,32);
    align32(b);
    const std::size_t first = b.size();
    b.insert(b.end(), 8, 0x11);
    while (b.size() < first + 32) b.push_back(0);
    b.insert(b.end(), 6 * 4, 0x22);

    embx::gguf::Document doc; std::string error;
    REQUIRE(embx::gguf::parse(b, doc, error));
    REQUIRE(doc.tensors.size() == 2);
    REQUIRE(doc.tensors[1].offset == 32);

    embx::runtime::ArrayView a, c;
    REQUIRE(embx::gguf::makeArrayView(doc, 0, b.data(), b.size(), a, error));
    REQUIRE(embx::gguf::makeArrayView(doc, 1, b.data(), b.size(), c, error));
    REQUIRE(a.buffer.data == b.data() + doc.tensorDataOffset);
    REQUIRE(c.buffer.data == b.data() + doc.tensorDataOffset + 32);
    REQUIRE(a.buffer.size == 8);
    REQUIRE(c.buffer.size == 24);
}

TEST_CASE("GGUF adapter rejects unsupported version and tensor-size overflow") {
    auto unsupported = fixture();
    unsupported[4] = 4; unsupported[5] = unsupported[6] = unsupported[7] = 0;
    embx::gguf::Document doc; std::string error;
    REQUIRE_FALSE(embx::gguf::parse(unsupported, doc, error));

    std::vector<std::uint8_t> b{'G','G','U','F'};
    u32(b,3); u64(b,1); u64(b,0);
    str(b,"overflow"); u32(b,2);
    u64(b, std::numeric_limits<std::uint64_t>::max()); u64(b,2);
    u32(b,0); u64(b,0); align32(b);
    b.resize(b.size() + 64, 0);
    REQUIRE(embx::gguf::parse(b, doc, error));
    embx::runtime::ArrayView view;
    REQUIRE_FALSE(embx::gguf::makeArrayView(doc, 0, b.data(), b.size(), view, error));
}

TEST_CASE("GGUF adapter rejects tensor data that does not fit the file") {
    auto bytes = fixture();
    bytes.resize(128 + 8);
    embx::gguf::Document doc; std::string error;
    REQUIRE(embx::gguf::parse(bytes, doc, error));
    embx::runtime::ArrayView view;
    REQUIRE_FALSE(embx::gguf::makeArrayView(doc, 0, bytes.data(), bytes.size(), view, error));
}

TEST_CASE("GGUF adapter accepts nested metadata arrays without leaking them into EmbX core") {
    std::vector<std::uint8_t> b{'G','G','U','F'};
    u32(b,3); u64(b,1); u64(b,1);
    str(b,"general.tags"); u32(b,9); u32(b,9); u64(b,2); u32(b,8); u64(b,1); str(b,"x"); u32(b,8); u64(b,1); str(b,"y");
    str(b,"tensor"); u32(b,1); u64(b,1); u32(b,0); u64(b,0); align32(b); u32(b,7);
    embx::gguf::Document doc; std::string error;
    REQUIRE(embx::gguf::parse(b, doc, error));
    REQUIRE(doc.metadata.size() == 1);
    REQUIRE(doc.tensors.size() == 1);
}
