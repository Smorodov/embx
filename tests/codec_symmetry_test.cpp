#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include <cstdint>
#include <cstring>
#include <vector>

using Value = embx::value::Value;
using Object = Value::Object;

static std::unique_ptr<embx::plan::Expr> lit(const char* s) {
    auto e = std::make_unique<embx::plan::Expr>();
    e->kind = embx::core::ExprKind::Literal;
    e->text = s;
    return e;
}
static embx::plan::Type primitive(const char* name) {
    embx::plan::Type t;
    t.kind = embx::core::TypeKind::Primitive;
    t.name = name;
    return t;
}
static std::unique_ptr<embx::plan::Field> field(const char* name, embx::plan::Type t,
                                                 embx::plan::Endian e = embx::plan::Endian::Little) {
    auto f = std::make_unique<embx::plan::Field>();
    f->name = name; f->type = std::move(t); f->endian = e;
    return f;
}

TEST_CASE("decoder and encoder preserve supported scalar and dynamic values", "[codec_symmetry]") {
    embx::plan::Module p;
    embx::plan::Struct s; s.name = "RoundTrip"; s.endian = embx::plan::Endian::Little;
    s.members.push_back(field("u8", primitive("u8")));
    s.members.push_back(field("u16", primitive("u16"), embx::plan::Endian::Big));
    s.members.push_back(field("u32", primitive("u32")));
    s.members.push_back(field("i8", primitive("i8")));
    s.members.push_back(field("i32", primitive("i32"), embx::plan::Endian::Big));
    s.members.push_back(field("f32", primitive("f32")));
    s.members.push_back(field("f64", primitive("f64"), embx::plan::Endian::Big));

    auto bytes = field("payload", primitive("bytes"));
    bytes->type.dimensions.push_back(embx::core::Dimension::fixed(lit("3")));
    s.members.push_back(std::move(bytes));

    auto text = field("text", primitive("string"));
    text->type.dimensions.push_back(embx::core::Dimension::fixed(lit("2")));
    s.members.push_back(std::move(text));

    auto arr = field("items", primitive("u16"));
    arr->type.dimensions.push_back(embx::core::Dimension::fixed(lit("2")));
    s.members.push_back(std::move(arr));

    auto dyn = field("tail", primitive("bytes"));
    dyn->type.dimensions.push_back(embx::core::Dimension::remaining()); // bytes[*]: consume the remaining bounded input
    s.members.push_back(std::move(dyn));

    embx::test::addStruct(p, std::move(s));

    Object root;
    root["u8"] = Value(uint64_t(0x7f));
    root["u16"] = Value(uint64_t(0x1234));
    root["u32"] = Value(uint64_t(0x89abcdef));
    root["i8"] = Value(int64_t(-7));
    root["i32"] = Value(int64_t(-123456));
    root["f32"] = Value(double(1.5));
    root["f64"] = Value(double(-2.25));
    root["payload"] = Value(Value::Bytes{0xaa, 0xbb, 0xcc});
    root["text"] = Value(std::string("OK"));
    Value::Array items; items.emplace_back(Value(uint64_t(0x0102))); items.emplace_back(Value(uint64_t(0x0304)));
    root["items"] = Value(std::move(items));
    root["tail"] = Value(Value::Bytes{0x10, 0x20, 0x30, 0x40});

    embx::encoder::Engine enc(p);
    auto er = enc.encode("RoundTrip", Value(root));
    REQUIRE(er.success);

    embx::decoder::Options opt; opt.requireFullInput = true;
    embx::decoder::Engine dec(p, opt);
    auto dr = dec.decode("RoundTrip", er.data);
    REQUIRE(dr.success);
    REQUIRE(dr.consumed == er.data.size());
    REQUIRE(embx::value::equal(dr.value, Value(root)));
}

TEST_CASE("decoder and encoder support their explicit callback directions", "[codec_symmetry]") {
    embx::runtime::CallbackRegistry dregistry;
    dregistry.add("on_decode_consume_one", [](const std::string&, embx::runtime::Reader& r,
                                   const embx::runtime::Environment&,
                                   const std::vector<embx::runtime::Value>&,
                                   std::string& err) {
        if (r.remaining() < 1) { err = "need one byte"; return false; }
        (void)r.uint(1); return true;
    });

    embx::plan::Module dp;
    embx::plan::Struct ds; ds.name = "DecodePacket";
    auto dcb = std::make_unique<embx::plan::Callback>();
    dcb->name = "on_decode_consume_one";
    dcb->direction = embx::core::CallbackDirection::Decode;
    ds.members.push_back(std::move(dcb));
    embx::test::addStruct(dp, std::move(ds));

    embx::decoder::Engine dec(dp);
    dec.callbacks(&dregistry);
    auto dr = dec.decode("DecodePacket", {0x42});
    REQUIRE(dr.success);
    REQUIRE(dr.consumed == 1);

    embx::runtime::CallbackRegistry eregistry;
    bool invoked = false;
    eregistry.add("on_encode_emit_one", [&](const std::string&, std::vector<uint8_t>& out, size_t& pos,
                                             const embx::runtime::Environment&,
                                             const std::vector<embx::runtime::Value>&,
                                             std::string&) {
        invoked = true;
        if (pos >= out.size()) out.resize(pos + 1);
        out[pos++] = 0xA5;
        return true;
    });

    embx::plan::Module ep;
    embx::plan::Struct es; es.name = "EncodePacket";
    auto ecb = std::make_unique<embx::plan::Callback>();
    ecb->name = "on_encode_emit_one";
    ecb->direction = embx::core::CallbackDirection::Encode;
    es.members.push_back(std::move(ecb));
    embx::test::addStruct(ep, std::move(es));

    embx::encoder::Options eopt; eopt.callbacks = &eregistry;
    embx::encoder::Engine enc(ep, eopt);
    auto er = enc.encode("EncodePacket", Value(Object{}));
    REQUIRE(er.success);
    REQUIRE(invoked);
    REQUIRE(er.data == std::vector<uint8_t>{0xA5});
}
