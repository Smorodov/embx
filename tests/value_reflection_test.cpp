#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "value/Value.h"
#include "diagnostics/Diagnostic.h"
#include "reflection/Reflection.h"
#include "plan/Plan.h"
#include <iostream>

TEST_CASE("value reflection", "[value_reflection]") {
    using embx::value::Value;
    Value::Object o{{"z",Value(uint64_t(2))},{"a",Value(std::string("x"))}};
    Value v(o);
    REQUIRE(v.kind()==Value::Kind::Object);
    REQUIRE(v.at("a").typeName()==std::string("string"));
    REQUIRE(v.debugString()=="{\"a\":\"x\",\"z\":2}");
    REQUIRE(embx::value::equal(v,Value(o)));

    embx::plan::Module p;
    embx::plan::Struct s; s.name="Packet";
    auto f=std::make_unique<embx::plan::Field>(); f->name="value"; f->type.name="u16"; s.members.push_back(std::move(f));
    embx::test::addStruct(p, std::move(s));
    auto r=embx::reflection::inspectStruct(p,"Packet");
    REQUIRE((r && r->members.size()==1 && r->members[0].name=="value"));
    REQUIRE(embx::reflection::kindName(r->members[0].kind)==std::string("field"));
    auto d=embx::diagnostics::Diagnostic{embx::diagnostics::Code::MissingField,"missing","Packet.value",3,0,0,false};
    REQUIRE(embx::diagnostics::format(d).find("Packet.value")!=std::string::npos);
    std::cout<<"Value/reflection tests passed\n";
}
