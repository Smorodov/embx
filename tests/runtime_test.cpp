#include <catch2/catch_test_macros.hpp>
#include "runtime/Runtime.h"
#include <iostream>
#include <stdexcept>
TEST_CASE("runtime", "[runtime]") {
    std::vector<uint8_t> d={0x78,0x56,0x12,0x34,1,2,3,0xF0,0x80,0x7F,0x01,0x02,0x03,0x04};
    embx::runtime::Reader r(d);
    r.endian(embx::runtime::Endian::Little); REQUIRE(r.uint(2)==0x5678);
    r.endian(embx::runtime::Endian::Big); REQUIRE(r.uint(2)==0x1234);
    r.seek(4); auto b=r.bytes(3); REQUIRE((b.size()==3&&b[2]==3));
    r.seek(1); r.align(4); REQUIRE(r.pos()==4);
    r.pushLimit(3); REQUIRE(r.remaining()==3); REQUIRE(r.bytes(3).size()==3); r.popLimit();
    r.seek(7); r.endian(embx::runtime::Endian::Big); REQUIRE(r.sint(1)==-16);
    r.seek(8); REQUIRE(r.sint(1)==-128);
    r.seek(7); r.pushLimit(1); bool threw=false; try{r.bytes(2);}catch(const std::out_of_range&){threw=true;} REQUIRE(threw); r.popLimit();
    r.seek(9); r.endian(embx::runtime::Endian::Big); r.beginBits(12); REQUIRE(r.bits(4)==0x7); REQUIRE(r.bits(8)==0xF0); r.endBits(); REQUIRE(r.pos()==11);
    std::vector<uint8_t> bd={0xAB,0xC0}; embx::runtime::Reader br(bd); br.endian(embx::runtime::Endian::Big); br.beginBits(12); REQUIRE(br.bits(4)==0xA); REQUIRE(br.bits(8)==0xBC); br.endBits(); REQUIRE(br.pos()==2);
    std::vector<uint8_t> ld={0xAB,0xCD}; embx::runtime::Reader lr(ld); lr.endian(embx::runtime::Endian::Little); lr.beginBits(12); REQUIRE(lr.bits(4)==0xC); REQUIRE(lr.bits(8)==0xDA); lr.endBits(); REQUIRE(lr.pos()==2);
    r.seek(0); r.pushLimit(2); threw=false; try{r.seek(3);}catch(const std::out_of_range&){threw=true;} REQUIRE(threw); r.popLimit();

    std::vector<uint8_t> fp={0x3f,0x80,0x00,0x00,0x3f,0xf0,0x00,0x00,0x00,0x00,0x00,0x00};
    embx::runtime::Reader fr(fp); fr.endian(embx::runtime::Endian::Big);
    REQUIRE(fr.f32()==1.0f); REQUIRE(fr.f64()==1.0);

    embx::runtime::CallbackRegistry registry;
    registry.add("ok", [](const std::string&, embx::runtime::Reader& rr, const embx::runtime::Environment& env, const std::vector<embx::runtime::Value>& args, std::string&){ REQUIRE(env.at("N") == embx::runtime::Value(int64_t(7))); REQUIRE((args.size()==1 && args[0] == embx::runtime::Value(int64_t(3)))); rr.seek(2); return true; });
    registry.add("fail", [](const std::string&, embx::runtime::Reader& rr, const embx::runtime::Environment&, const std::vector<embx::runtime::Value>&, std::string& e){ rr.seek(3); e="expected failure"; return false; });
    embx::runtime::Environment cbEnv; cbEnv["N"] = int64_t(7); std::vector<embx::runtime::Value> cbArgs{int64_t(3)};
    r.seek(0);
    auto ok=registry.call("ok",r,cbEnv,cbArgs); REQUIRE((ok.invoked&&ok.success&&r.pos()==2));
    r.seek(1); auto fail=registry.call("fail",r); REQUIRE((fail.invoked&&!fail.success&&fail.error=="expected failure"&&r.pos()==1));
    auto missing=registry.call("missing",r); REQUIRE((!missing.invoked&&!missing.success));

    std::cout<<"runtime tests passed\n";
}


TEST_CASE("one callback registry serves both codec directions") {
    embx::runtime::CallbackRegistry registry;
    registry.add("decode_hook", [](const std::string&, embx::runtime::Reader& r, const embx::runtime::Environment&, const std::vector<embx::runtime::Value>& args, std::string& err) {
        if (args.size() != 1 || !std::holds_alternative<int64_t>(args[0])) { err = "bad decode args"; return false; }
        (void)r.uint(1); return true;
    });
    registry.add("encode_hook", [](const std::string&, std::vector<uint8_t>& out, size_t& pos, const embx::runtime::Environment&, const std::vector<embx::runtime::Value>& args, std::string& err) {
        if (args.size() != 1 || !std::holds_alternative<int64_t>(args[0])) { err = "bad encode args"; return false; }
        if (pos >= out.size()) {
            out.resize(pos + 1);
        }
        out[pos++] = 0x5A;
        return true;
    });
    embx::runtime::Environment env;
    std::vector<embx::runtime::Value> args{int64_t(9)};
    std::vector<uint8_t> bytes{0x11};
    embx::runtime::Reader r(bytes);
    auto dr = registry.call("decode_hook", r, env, args);
    REQUIRE((dr.invoked && dr.success && r.pos() == 1));
    std::vector<uint8_t> out; size_t pos = 0;
    auto er = registry.call("encode_hook", out, pos, env, args);
    REQUIRE((er.invoked && er.success && pos == 1 && out == std::vector<uint8_t>{0x5A}));
}
