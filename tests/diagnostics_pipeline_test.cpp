#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"
#include "diagnostics/Diagnostic.h"
#include <iostream>

using namespace embx;

static plan::Module makePlan() {
    plan::Module p;
    plan::Struct s; s.name="Packet";
    auto h=std::make_unique<plan::Field>(); h->name="header"; h->type.kind=core::TypeKind::Primitive; h->type.name="u8"; s.members.push_back(std::move(h));
    auto payload=std::make_unique<plan::Field>(); payload->name="payload"; payload->type.kind=core::TypeKind::Bytes; payload->type.name="bytes";
    payload->staticLength=4; s.members.push_back(std::move(payload));
    embx::test::addStruct(p, std::move(s));
    return p;
}

TEST_CASE("diagnostics pipeline", "[diagnostics_pipeline]") {
    auto p=makePlan();

    decoder::Engine d(p);
    auto dr=d.decode("Packet", {0x7f, 0x01});
    REQUIRE(!dr.success);
    REQUIRE(dr.diagnostic.code==diagnostics::Code::InputTruncated);
    REQUIRE(dr.diagnostic.path=="Packet.payload");
    REQUIRE(dr.diagnostic.byteOffset==1);
    REQUIRE(dr.error.find("beyond")!=std::string::npos);
    REQUIRE(diagnostics::format(dr.diagnostic).find("Packet.payload")!=std::string::npos);

    encoder::Value::Object good{{"header", value::Value(uint64_t(1))}, {"payload", value::Value(value::Value::Bytes{1,2,3,4})}};
    auto er=encoder::Engine(p).encode("Packet", value::Value(good));
    REQUIRE((er.success && er.data.size()==5));

    encoder::Value::Object bad{{"header", value::Value(uint64_t(1))}};
    auto mr=encoder::Engine(p).encode("Packet", value::Value(bad));
    REQUIRE(!mr.success);
    REQUIRE(mr.diagnostic.code==diagnostics::Code::MissingField);
    REQUIRE(mr.diagnostic.path=="Packet.payload");
    REQUIRE(mr.diagnostic.byteOffset==1);

    auto ur=encoder::Engine(p).encode("NoSuchStruct", value::Value{});
    REQUIRE(!ur.success);
    REQUIRE(ur.diagnostic.code==diagnostics::Code::UnknownStruct);
    REQUIRE(ur.diagnostic.path=="NoSuchStruct");

    std::cout << "Diagnostics pipeline tests passed\n";
}
