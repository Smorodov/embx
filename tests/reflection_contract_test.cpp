#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "reflection/Reflection.h"

TEST_CASE("reflection preserves executable structure, attributes and documentation", "[reflection_contract]") {
    embx::plan::Module p;
    p.documentation = "Module docs";
    embx::plan::Struct s; s.name = "Packet"; s.documentation = "Packet docs";
    embx::core::Attribute packed; packed.name = "packed"; packed.hasValue = false;
    s.attributes.push_back(packed);

    auto f = std::make_unique<embx::plan::Field>();
    f->name = "value"; f->type.name = "u16"; f->documentation = "Value docs";
    embx::core::Attribute width; width.name = "width"; width.value = "16"; width.hasValue = true;
    f->attributes.push_back(width);
    s.members.push_back(std::move(f));

    auto bits = std::make_unique<embx::plan::Bits>();
    bits->totalBits = 8; bits->storageBytes = 1;
    embx::plan::BitsField lo; lo.name = "lo"; lo.type.name = "u8"; lo.bits = 3; lo.documentation = "Low bits"; embx::core::Attribute loAttr; loAttr.name = "flagged"; lo.attributes.push_back(loAttr);
    embx::plan::BitsField hi; hi.name = "hi"; hi.type.name = "u8"; hi.bits = 5; hi.documentation = "High bits";
    bits->fields.push_back(std::move(lo)); bits->fields.push_back(std::move(hi));
    s.members.push_back(std::move(bits));

    auto cb = std::make_unique<embx::plan::Callback>();
    cb->name = "on_encode_value";
    cb->direction = embx::core::CallbackDirection::Encode;
    auto arg = std::make_unique<embx::plan::Expr>();
    arg->text = "value + 1";
    cb->args.push_back(std::move(arg));
    s.members.push_back(std::move(cb));

    embx::test::addStruct(p, std::move(s));

    embx::plan::CallbackDecl cd;
    cd.name = "on_encode_value";
    cd.direction = embx::core::CallbackDirection::Encode;
    p.callbacks.push_back(cd);

    auto m = embx::reflection::inspect(p);
    REQUIRE(m.documentation == "Module docs");
    REQUIRE(m.structNames.size() == 1);
    REQUIRE(m.structAttributes[0].size() == 1);
    REQUIRE(m.structAttributes[0][0] == "packed");
    REQUIRE(m.callbackDirections.size() == 1);
    REQUIRE(m.callbackDirections[0] == embx::core::CallbackDirection::Encode);

    auto rs = embx::reflection::inspectStruct(p, "Packet");
    REQUIRE(rs.has_value());
    REQUIRE(rs->documentation == "Packet docs");
    REQUIRE(rs->attributes.size() == 1);
    REQUIRE(rs->attributes[0] == "packed");
    REQUIRE(rs->members.size() == 3);
    REQUIRE(rs->members[0].documentation == "Value docs");
    REQUIRE(rs->members[0].attributes.size() == 1);
    REQUIRE(rs->members[0].attributes[0] == "width=16");
    REQUIRE(rs->members[1].kind == embx::reflection::Kind::Bits);
    REQUIRE((rs->members[1].children == std::vector<std::string>{"lo", "hi"}));
    REQUIRE(rs->members[1].nested.size() == 2);
    REQUIRE(rs->members[1].nested[0].documentation == "Low bits");
    REQUIRE(rs->members[1].nested[0].attributes.size() == 1);
    REQUIRE(rs->members[1].nested[0].attributes[0] == "flagged");
    REQUIRE(rs->members[1].nested[1].documentation == "High bits");
    REQUIRE(rs->members[1].nested[0].bits == 3);
    REQUIRE(rs->members[1].nested[1].bits == 5);
    auto rs2 = embx::reflection::inspectStruct(p, "Packet");
    REQUIRE(rs2.has_value());
    REQUIRE(rs2->members.size() == 3);
    REQUIRE(rs2->members[2].kind == embx::reflection::Kind::Callback);
    REQUIRE(rs2->members[2].name == "on_encode_value");
    REQUIRE(rs2->members[2].callbackDirection == embx::core::CallbackDirection::Encode);
    REQUIRE(rs2->members[2].callbackArgs == std::vector<std::string>{"value + 1"});
}

TEST_CASE("reflection exposes plan layout bounds and symbol identity", "[reflection_contract]") {
    embx::plan::Module p;
    embx::plan::Struct s; s.name = "Packet"; s.minSizeInBytes = 4; s.maxSizeInBytes = 12;
    auto f = std::make_unique<embx::plan::Field>();
    f->symbol = 7; f->name = "payload"; f->type.name = "bytes";
    f->type.dimensions.push_back(embx::core::Dimension::remaining());
    s.members.push_back(std::move(f));
    auto a = std::make_unique<embx::plan::FieldAlias>();
    a->symbol = 8; a->target = 7; a->name = "data"; a->targetName = "payload";
    s.members.push_back(std::move(a));
    embx::test::addStruct(p, std::move(s));
    auto r = embx::reflection::inspectStruct(p, "Packet");
    REQUIRE(r);
    REQUIRE(r->minSize == 4);
    REQUIRE(r->maxSize == 12);
    REQUIRE(r->layoutClass == embx::reflection::LayoutClass::Bounded);
    REQUIRE(r->members[0].symbol == 7);
    REQUIRE(r->members[0].layoutClass == embx::reflection::LayoutClass::Unbounded);
    REQUIRE(r->members[1].symbol == 8);
    REQUIRE(r->members[1].targetSymbol == 7);
}
