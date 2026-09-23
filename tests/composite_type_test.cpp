#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"
#include <cstdint>
#include <iostream>

using Value = embx::value::Value;
using Object = Value::Object;
using Array = Value::Array;

static Value u(uint64_t x) { return Value{x}; }
static const Value& at(const Value& v, const char* n) { return std::get<Object>(v.data).at(n); }
static uint64_t uv(const Value& v) { return std::get<uint64_t>(v.data); }

static std::unique_ptr<embx::plan::Expr> lit(const char* text) {
    auto e = std::make_unique<embx::plan::Expr>();
    e->kind = embx::core::ExprKind::Literal;
    e->text = text;
    return e;
}

static embx::plan::Type primitive(const char* name) {
    embx::plan::Type t;
    t.kind = embx::core::TypeKind::Primitive;
    t.name = name;
    return t;
}

static embx::plan::Type named(const char* name, embx::core::SymbolId id) {
    embx::plan::Type t;
    t.kind = embx::core::TypeKind::Named;
    t.name = name;
    t.reference.id = id;
    return t;
}

TEST_CASE("composite type", "[composite_type]") {
    embx::plan::Module p;

    embx::plan::Struct item;
    item.name = "Item";
    {
        auto f = std::make_unique<embx::plan::Field>();
        f->name = "id"; f->type = primitive("u16");
        item.members.push_back(std::move(f));
    }
    {
        auto f = std::make_unique<embx::plan::Field>();
        f->name = "value"; f->type = primitive("u8");
        item.members.push_back(std::move(f));
    }
    const auto itemId = embx::test::addStruct(p, std::move(item));

    embx::plan::Alias alias;
    alias.name = "ItemAlias";
    alias.target = named("Item", itemId);
    embx::test::addAlias(p, std::move(alias));

    embx::plan::Struct packet;
    packet.name = "Packet";
    {
        auto f = std::make_unique<embx::plan::Field>();
        f->name = "items";
        f->type = named("Item", itemId);
        f->type.dimensions.push_back(embx::core::Dimension::fixed(lit("3")));
        packet.members.push_back(std::move(f));
    }
    {
        auto f = std::make_unique<embx::plan::Field>();
        f->name = "aliases";
        f->type = named("Item", itemId);
        f->type.dimensions.push_back(embx::core::Dimension::fixed(lit("2")));
        packet.members.push_back(std::move(f));
    }
    auto packetId = embx::test::addStruct(p, std::move(packet));
    (void)packetId;

    Object i1{{"id",u(0x1001)}, {"value",u(11)}};
    Object i2{{"id",u(0x1002)}, {"value",u(22)}};
    Object i3{{"id",u(0x1003)}, {"value",u(33)}};
    Object a1{{"id",u(0x2001)}, {"value",u(44)}};
    Object a2{{"id",u(0x2002)}, {"value",u(55)}};

    Array items; items.emplace_back(std::move(i1)); items.emplace_back(std::move(i2)); items.emplace_back(std::move(i3));
    Array aliases; aliases.emplace_back(std::move(a1)); aliases.emplace_back(std::move(a2));
    Object root{{"items",std::move(items)}, {"aliases",std::move(aliases)}};

    embx::encoder::Engine enc(p);
    auto er = enc.encode("Packet", Value{root});
    REQUIRE(er.success);
    const std::vector<uint8_t> expected = {
        0x01,0x10,11, 0x02,0x10,22, 0x03,0x10,33,
        0x01,0x20,44, 0x02,0x20,55
    };
    REQUIRE(er.data == expected);

    embx::decoder::Options opt;
    opt.requireFullInput = true;
    embx::decoder::Engine dec(p, opt);
    auto dr = dec.decode("Packet", er.data);
    REQUIRE(dr.success);
    REQUIRE(dr.consumed == er.data.size());

    const auto& decodedItems = std::get<Array>(at(dr.value, "items").data);
    REQUIRE(decodedItems.size() == 3);
    REQUIRE((uv(at(decodedItems[0], "id")) == 0x1001 && uv(at(decodedItems[0], "value")) == 11));
    REQUIRE((uv(at(decodedItems[2], "id")) == 0x1003 && uv(at(decodedItems[2], "value")) == 33));

    const auto& decodedAliases = std::get<Array>(at(dr.value, "aliases").data);
    REQUIRE(decodedAliases.size() == 2);
    REQUIRE((uv(at(decodedAliases[0], "id")) == 0x2001 && uv(at(decodedAliases[0], "value")) == 44));
    REQUIRE((uv(at(decodedAliases[1], "id")) == 0x2002 && uv(at(decodedAliases[1], "value")) == 55));

    std::cout << "Composite type tests passed\n";
}

TEST_CASE("multidimensional arrays preserve runtime shape", "[composite_type]") {
    embx::plan::Module p;
    embx::plan::Struct s;
    s.name = "Matrix";
    auto f = std::make_unique<embx::plan::Field>();
    f->name = "values";
    f->type = primitive("u8");
    f->type.dimensions.push_back(embx::core::Dimension::fixed(lit("2")));
    f->type.dimensions.push_back(embx::core::Dimension::fixed(lit("3")));
    s.members.push_back(std::move(f));
    embx::test::addStruct(p, std::move(s));

    Array row1; row1.emplace_back(u(1)); row1.emplace_back(u(2)); row1.emplace_back(u(3));
    Array row2; row2.emplace_back(u(4)); row2.emplace_back(u(5)); row2.emplace_back(u(6));
    Array matrix; matrix.emplace_back(std::move(row1)); matrix.emplace_back(std::move(row2));
    Object root{{"values", std::move(matrix)}};

    embx::encoder::Engine enc(p);
    auto er = enc.encode("Matrix", Value{root});
    REQUIRE(er.success);
    const std::vector<uint8_t> expected = {1,2,3,4,5,6};
    REQUIRE(er.data == expected);

    embx::decoder::Options opt;
    opt.requireFullInput = true;
    embx::decoder::Engine dec(p, opt);
    auto dr = dec.decode("Matrix", er.data);
    REQUIRE(dr.success);
    REQUIRE(dr.consumed == er.data.size());

    const auto& outer = std::get<Array>(at(dr.value, "values").data);
    REQUIRE(outer.size() == 2);
    const auto& first = std::get<Array>(outer[0].data);
    const auto& second = std::get<Array>(outer[1].data);
    REQUIRE(first.size() == 3);
    REQUIRE(second.size() == 3);
    REQUIRE(uv(first[0]) == 1);
    REQUIRE(uv(first[2]) == 3);
    REQUIRE(uv(second[0]) == 4);
    REQUIRE(uv(second[2]) == 6);
}
