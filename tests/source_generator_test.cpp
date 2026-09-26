#include <catch2/catch_test_macros.hpp>
#include "codegen/SourceGenerator.h"
#include "parser/ParserDriver.h"
#include <cstdio>
#include <fstream>
#include <string>

static std::unique_ptr<embx::ast::Module> parse(const std::string& path, std::string& error) {
    auto a = embx::parser::parseFile(path, error);
    REQUIRE(a);
    REQUIRE(error.empty());
    return a;
}

static std::string generate(const embx::ast::Module& m) {
    std::string out, error;
    REQUIRE(embx::codegen::generateSource(m, out, error));
    REQUIRE(error.empty());
    REQUIRE_FALSE(out.empty());
    return out;
}

TEST_CASE("source generator is deterministic and idempotent") {
    const std::string input = "source_generator_roundtrip.embx";
    const std::string output = "source_generator_roundtrip.generated.embx";
    std::ofstream f(input);
    f << "/// module docs\n"
      << "namespace demo::wire;\n"
      << "attribute flag;\n"
      << "@ endian little\n"
      << "const N = 3;\n"
      << "param limit: u32;\n"
      << "type Payload = bytes[*];\n"
      << "enum Kind: u8 { A = 1, B = 2, }\n"
      << "callback on_decode_packet(ctx);\n"
      << "struct Packet big {\n"
      << "  requires N > 0;\n"
      << "  value: u16 little;\n"
      << "  data: bytes[N];\n"
      << "  bits { flag: u8 (1); }\n"
      << "  if (value == 1) { let x = value + N; } else { alias y = value; }\n"
      << "  variant body by value { 1: Payload; default: { align(4); } }\n"
      << "  block payload [limit] { at($next) { callback on_decode_packet(value); } }\n"
      << "}\n";
    f.close();

    std::string e1;
    auto a1 = parse(input, e1);
    const auto s1 = generate(*a1);
    std::ofstream g(output); g << s1; g.close();

    std::string e2;
    auto a2 = parse(output, e2);
    REQUIRE(e2.empty());
    const auto s2 = generate(*a2);
    REQUIRE(s1 == s2);

    std::remove(input.c_str());
    std::remove(output.c_str());
}

TEST_CASE("source generator round-trips current examples") {
    const char* files[] = {
        "../examples/common.embx",
        "../examples/container.embx",
        "../examples/conditional.embx",
        "../examples/ipv4.embx",
        "../examples/metadata.embx",
        "../examples/midi.embx",
        "../examples/nested_types.embx",
        "../examples/next.embx",
        "../examples/runtime_params.embx",
        "../course/01_first_format/message.embx",
        "../course/02_integers_byte_order/message.embx",
        "../course/03_bit_fields/flags.embx",
        "../course/04_arrays_dynamic_dimensions/arrays.embx",
        "../course/05_nested_structures/nested.embx",
        "../course/06_variants_conditionals/variants.embx",
        "../course/07_offsets_alignment/offsets_alignment.embx",
        "../course/08_symbol_dependencies_runtime_parameters/parameters.embx",
        "../course/09_virtual_fields_aliases/projected.embx",
        "../course/10_callbacks_transforms/callbacks_transforms.embx",
        "../course/11_ipv4/ipv4.embx",
        "../course/12_terminated_sequences/terminated_sequences.embx",
        "../course/14_tlv/tlv.embx"
    };
    for (const char* file : files) {
        DYNAMIC_SECTION(file) {
            std::string error;
            auto a = embx::parser::parseFile(file, error);
            REQUIRE(a);
            REQUIRE(error.empty());
            std::string s1;
            REQUIRE(embx::codegen::generateSource(*a, s1, error));
            REQUIRE(error.empty());
            const std::string tmp = std::string(file) + ".generated.embx";
            std::ofstream out(tmp); out << s1; out.close();
            auto b = embx::parser::parseFile(tmp, error);
            REQUIRE(b);
            REQUIRE(error.empty());
            std::string s2;
            REQUIRE(embx::codegen::generateSource(*b, s2, error));
            REQUIRE(error.empty());
            REQUIRE(s1 == s2);
            std::remove(tmp.c_str());
        }
    }
}

namespace {
struct UnsupportedMember final : embx::ast::Member {};
}

TEST_CASE("source generator rejects incomplete and unsupported AST nodes") {
    SECTION("terminated sequence without max payload") {
        embx::ast::Module m;
        embx::ast::TypeAlias a;
        a.name = "Broken";
        a.target.name = "bytes";
        a.target.terminator = {0x00};
        m.aliases.push_back(std::move(a));
        m.order.push_back({embx::ast::TopLevelRef::Kind::Alias, 0});

        std::string out, error;
        REQUIRE_FALSE(embx::codegen::generateSource(m, out, error));
        REQUIRE(out.empty());
        REQUIRE(error.find("missing max payload") != std::string::npos);
    }

    SECTION("unsupported member kind") {
        embx::ast::Module m;
        auto s = std::make_unique<embx::ast::Struct>();
        s->name = "Broken";
        s->members.push_back(std::make_unique<UnsupportedMember>());
        m.structs.push_back(std::move(s));
        m.order.push_back({embx::ast::TopLevelRef::Kind::Struct, 0});

        std::string out, error;
        REQUIRE_FALSE(embx::codegen::generateSource(m, out, error));
        REQUIRE(out.empty());
        REQUIRE(error == "source generator: unsupported AST member kind");
    }
}
