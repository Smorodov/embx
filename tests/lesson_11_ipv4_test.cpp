#include <catch2/catch_test_macros.hpp>
#include "compiler/Compiler.h"
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"
#include "value/Value.h"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

using Value = embx::value::Value;
using Object = Value::Object;

static const Value& field(const Value& v, const char* name) {
    return std::get<Object>(v.data).at(name);
}

static std::uint64_t u(const Value& v) {
    REQUIRE(v.kind() == Value::Kind::Unsigned);
    return std::get<std::uint64_t>(v.data);
}

static std::uint16_t ipv4Checksum(const std::vector<std::uint8_t>& header) {
    REQUIRE(header.size() == 20);
    std::uint32_t sum = 0;
    for (std::size_t i = 0; i < header.size(); i += 2)
        sum += (static_cast<std::uint16_t>(header[i]) << 8) | header[i + 1];
    while (sum >> 16)
        sum = (sum & 0xffffu) + (sum >> 16);
    return static_cast<std::uint16_t>(~sum);
}

TEST_CASE("Lesson 11 IPv4 header decodes and re-encodes the real fixture", "[course][ipv4]") {
    std::string error;
    auto compilation = embx::compiler::compileFile("../course/11_ipv4/ipv4.embx", error);
    REQUIRE(compilation);
    REQUIRE(error.empty());
    REQUIRE(compilation->plan);

    const auto it = std::find_if(compilation->plan->structs.begin(), compilation->plan->structs.end(),
        [](const auto& s) { return s.name == "net::ipv4::Header"; });
    REQUIRE(it != compilation->plan->structs.end());
    REQUIRE(it->staticSize.has_value());
    REQUIRE(*it->staticSize == 20);
    REQUIRE(it->minSizeInBytes == 20);
    REQUIRE(it->maxSizeInBytes.has_value());
    REQUIRE(*it->maxSizeInBytes == 20);

    std::ifstream in("../course/11_ipv4/ipv4_header.bin", std::ios::binary);
    REQUIRE(in);
    const std::vector<std::uint8_t> bytes(
        std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{});
    REQUIRE(bytes.size() == 20);
    REQUIRE(ipv4Checksum(bytes) == 0);

    embx::decoder::Options options;
    options.requireFullInput = true;
    embx::decoder::Engine decoder(*compilation->plan, options);
    const auto decoded = decoder.decode("net::ipv4::Header", bytes);
    REQUIRE(decoded.success);
    REQUIRE(decoded.consumed == bytes.size());

    REQUIRE(u(field(decoded.value, "version")) == 4);
    REQUIRE(u(field(decoded.value, "ihl")) == 5);
    REQUIRE(u(field(decoded.value, "dscp")) == 0);
    REQUIRE(u(field(decoded.value, "total_length")) == 20);
    REQUIRE(u(field(decoded.value, "identification")) == 0x1234);
    REQUIRE(u(field(decoded.value, "flags_fragment")) == 0x4000);
    REQUIRE(u(field(decoded.value, "ttl")) == 64);
    REQUIRE(u(field(decoded.value, "protocol")) == 6);
    REQUIRE(u(field(decoded.value, "checksum")) == 0x3c79);

    REQUIRE(std::get<Value::Bytes>(field(decoded.value, "source").data) == Value::Bytes({192, 0, 2, 1}));
    REQUIRE(std::get<Value::Bytes>(field(decoded.value, "destination").data) == Value::Bytes({198, 51, 100, 2}));

    embx::encoder::Engine encoder(*compilation->plan);
    const auto encoded = encoder.encode("net::ipv4::Header", decoded.value);
    REQUIRE(encoded.success);
    REQUIRE(encoded.data == bytes);
}
