#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "encoder/Encoder.h"

using Value = embx::value::Value;

TEST_CASE("encoder invokes explicit on_encode callbacks") {
    embx::plan::Module p;
    embx::plan::Struct s; s.name="Packet";
    auto cb=std::make_unique<embx::plan::Callback>(); cb->name="on_encode"; cb->direction=embx::core::CallbackDirection::Encode; s.members.push_back(std::move(cb));
    embx::test::addStruct(p, std::move(s));

    embx::runtime::CallbackRegistry registry;
    bool invoked=false;
    registry.add("on_encode", [&](const std::string&, std::vector<uint8_t>& out, size_t& pos, const embx::runtime::Environment&, const std::vector<embx::runtime::Value>&, std::string&){
        invoked=true; out.resize(pos+1); out[pos++]=0xA5; return true;
    });
    embx::encoder::Options opt; opt.callbacks=&registry;
    embx::encoder::Engine e(p,opt);
    Value::Object o;
    auto r=e.encode("Packet",Value{o});
    REQUIRE(r.success);
    REQUIRE(invoked);
    REQUIRE(r.data==std::vector<uint8_t>{0xA5});
}

TEST_CASE("encoder rejects a callback with the opposite explicit direction") {
    embx::plan::Module p;
    embx::plan::Struct s; s.name="Packet";
    auto cb=std::make_unique<embx::plan::Callback>(); cb->name="on_decode"; cb->direction=embx::core::CallbackDirection::Decode; s.members.push_back(std::move(cb));
    embx::test::addStruct(p, std::move(s));
    embx::encoder::Engine e(p);
    Value::Object o;
    auto r=e.encode("Packet",Value{o});
    REQUIRE_FALSE(r.success);
    REQUIRE(r.error.find("callback direction is not valid for encoder")!=std::string::npos);
}


TEST_CASE("encoder passes evaluated callback arguments through the runtime callback contract") {
    embx::plan::Module p;
    embx::plan::Struct s; s.name="Packet";
    auto cb=std::make_unique<embx::plan::Callback>();
    cb->name="on_encode_args";
    cb->direction=embx::core::CallbackDirection::Encode;
    auto arg=std::make_unique<embx::plan::Expr>();
    arg->kind=embx::core::ExprKind::Literal;
    arg->text="42";
    cb->args.push_back(std::move(arg));
    s.members.push_back(std::move(cb));
    embx::test::addStruct(p, std::move(s));

    embx::runtime::CallbackRegistry registry;
    registry.add("on_encode_args", [](const std::string&, std::vector<uint8_t>& out, size_t& pos,
                                       const embx::runtime::Environment&,
                                       const std::vector<embx::runtime::Value>& args, std::string& error) {
        if(args.size()!=1 || !std::holds_alternative<int64_t>(args[0])) { error="unexpected callback arguments"; return false; }
        if(std::get<int64_t>(args[0])!=42) { error="unexpected callback value"; return false; }
        if(pos>=out.size()) out.resize(pos+1);
        out[pos++]=0x42;
        return true;
    });
    embx::encoder::Options opt; opt.callbacks=&registry;
    embx::encoder::Engine e(p,opt);
    Value::Object o;
    auto r=e.encode("Packet",Value{o});
    REQUIRE(r.success);
    REQUIRE(r.data==std::vector<uint8_t>{0x42});
}


TEST_CASE("encoder continues at the callback-advanced position") {
    embx::plan::Module p;
    embx::plan::Struct s; s.name="Packet";
    auto cb=std::make_unique<embx::plan::Callback>(); cb->name="on_encode"; cb->direction=embx::core::CallbackDirection::Encode; s.members.push_back(std::move(cb));
    auto f=std::make_unique<embx::plan::Field>(); f->name="tail"; f->type.name="u8"; f->type.kind=embx::core::TypeKind::Primitive; s.members.push_back(std::move(f));
    embx::test::addStruct(p, std::move(s));

    embx::runtime::CallbackRegistry registry;
    registry.add("on_encode", [](const std::string&, std::vector<uint8_t>& out, size_t& pos,
                                   const embx::runtime::Environment&, const std::vector<embx::runtime::Value>&, std::string&) {
        out.resize(pos + 2);
        out[pos++] = 0xA5;
        out[pos++] = 0x5A;
        return true;
    });
    embx::encoder::Options opt; opt.callbacks=&registry;
    embx::encoder::Engine e(p,opt);
    Value::Object o; o["tail"] = uint64_t(0xCC);
    auto r=e.encode("Packet",Value{o});
    REQUIRE(r.success);
    REQUIRE(r.data==std::vector<uint8_t>{0xA5,0x5A,0xCC});
}

TEST_CASE("failed encode callback is transactional at the callback boundary") {
    embx::plan::Module p;
    embx::plan::Struct s; s.name="Packet";
    auto f=std::make_unique<embx::plan::Field>(); f->name="head"; f->type.name="u8"; f->type.kind=embx::core::TypeKind::Primitive; s.members.push_back(std::move(f));
    auto cb=std::make_unique<embx::plan::Callback>(); cb->name="on_encode_fail"; cb->direction=embx::core::CallbackDirection::Encode; s.members.push_back(std::move(cb));
    embx::test::addStruct(p, std::move(s));

    embx::runtime::CallbackRegistry registry;
    registry.add("on_encode_fail", [](const std::string&, std::vector<uint8_t>& out, size_t& pos,
                                        const embx::runtime::Environment&, const std::vector<embx::runtime::Value>&, std::string& error) {
        out.resize(pos + 3, 0xEE);
        pos += 3;
        error = "intentional callback failure";
        return false;
    });
    embx::encoder::Options opt; opt.callbacks=&registry;
    embx::encoder::Engine e(p,opt);
    Value::Object o; o["head"] = uint64_t(0x11);
    auto r=e.encode("Packet",Value{o});
    REQUIRE_FALSE(r.success);
    REQUIRE(r.error.find("intentional callback failure")!=std::string::npos);
}
