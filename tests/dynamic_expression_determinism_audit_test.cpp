#include <catch2/catch_test_macros.hpp>
#include "ir/IrBuilder.h"
#include "parser/ParserDriver.h"
#include "plan/PlanBuilder.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

static void exprFingerprint(const embx::core::Expr* e, std::ostringstream& out) {
    if (!e) { out << "<null>"; return; }
    out << static_cast<int>(e->kind) << ':' << e->reference.id << ':'
        << e->op << ':' << e->text << '(';
    exprFingerprint(e->left.get(), out);
    out << ',';
    exprFingerprint(e->right.get(), out);
    out << ')';
}

static void typeFingerprint(const embx::core::Type& t, std::ostringstream& out) {
    out << static_cast<int>(t.kind) << ':' << t.name << ':' << t.reference.id << '[';
    for (const auto& d : t.dimensions) {
        out << static_cast<int>(d.kind) << ':';
        exprFingerprint(d.expression.get(), out);
        out << ';';
    }
    out << ']';
}

static void opFingerprint(const embx::plan::Op* op, std::ostringstream& out) {
    out << "O" << static_cast<int>(op->kind) << ':' << op->attributes.size() << ':' << op->documentation << '{';
    if (auto* f = dynamic_cast<const embx::plan::Field*>(op)) {
        out << "F:" << f->symbol << ':' << f->name << ':' << static_cast<int>(f->endian) << ':'
            << f->bits << ':' << f->assertion << ':' << (f->staticLength ? std::to_string(*f->staticLength) : "-") << ':'
            << (f->staticOffset ? std::to_string(*f->staticOffset) : "-") << ':'
            << (f->staticSize ? std::to_string(*f->staticSize) : "-");
        typeFingerprint(f->type, out);
    } else if (auto* b = dynamic_cast<const embx::plan::Block*>(op)) {
        out << "B:" << b->name << ':' << (b->staticSize ? std::to_string(*b->staticSize) : "-");
        exprFingerprint(b->size.get(), out);
        for (const auto& m : b->members) opFingerprint(m.get(), out);
    } else if (auto* a = dynamic_cast<const embx::plan::Align*>(op)) {
        out << "A:" << (a->staticAlignment ? std::to_string(*a->staticAlignment) : "-");
        exprFingerprint(a->alignment.get(), out);
    } else if (auto* a = dynamic_cast<const embx::plan::At*>(op)) {
        out << "T:" << (a->staticOffset ? std::to_string(*a->staticOffset) : "-");
        exprFingerprint(a->offset.get(), out);
        for (const auto& m : a->members) opFingerprint(m.get(), out);
    } else if (auto* v = dynamic_cast<const embx::plan::Variant*>(op)) {
        out << "V:" << v->name << ':' << v->cases.size() << ':' << v->hasDefault;
        exprFingerprint(v->discriminator.get(), out);
        for (const auto& c : v->cases) {
            if (const auto* sv = std::get_if<std::int64_t>(&c.tag)) out << "I" << *sv; else if (const auto* uv = std::get_if<std::uint64_t>(&c.tag)) out << "U" << *uv; else out << "?";
            if (c.type) typeFingerprint(*c.type, out); else out << "<members>";
            for (const auto& m : c.members) opFingerprint(m.get(), out);
        }
        if (v->defaultType) typeFingerprint(*v->defaultType, out);
        for (const auto& m : v->defaultMembers) opFingerprint(m.get(), out);
    } else if (auto* b = dynamic_cast<const embx::plan::Bits*>(op)) {
        out << "S:" << b->totalBits << ':' << b->storageBytes;
        for (const auto& f : b->fields) {
            out << f.symbol << ':' << f.name << ':' << f.bits << ':' << f.assertion;
            typeFingerprint(f.type, out);
        }
    } else if (auto* c = dynamic_cast<const embx::plan::Callback*>(op)) {
        out << "C:" << c->name << ':' << static_cast<int>(c->direction) << ':' << c->args.size();
        for (const auto& e : c->args) exprFingerprint(e.get(), out);
    }
    out << '}';
}

static std::string runtimeValueFingerprint(const embx::runtime::Value& v) {
    std::ostringstream out;
    out << v.index() << ':';
    std::visit([&](const auto& x) { out << x; }, v);
    return out.str();
}

static std::string fingerprint(const embx::plan::Module& p) {
    std::ostringstream out;
    out << p.documentation << ':' << static_cast<int>(p.endian) << '|';
    for (const auto& a : p.aliases) {
        out << "AL:" << a.symbol << ':' << a.name; typeFingerprint(a.target, out);
    }
    for (const auto& c : p.constants) out << "CO:" << c.symbol << ':' << c.name << ':' << runtimeValueFingerprint(c.value) << ':' << c.computed;
    for (const auto& e : p.enums) {
        out << "EN:" << e.symbol << ':' << e.name; typeFingerprint(e.underlying, out);
        for (const auto& i : e.items) out << "EI:" << i.symbol << ':' << i.name << ':' << runtimeValueFingerprint(i.value);
    }
    for (const auto& s : p.structs) {
        out << "ST:" << s.symbol << ':' << s.name << ':' << static_cast<int>(s.endian) << ':'
            << (s.staticSize ? std::to_string(*s.staticSize) : "-");
        for (const auto& m : s.members) opFingerprint(m.get(), out);
    }
    for (const auto& c : p.callbacks) out << "CB:" << c.symbol << ':' << c.name << ':' << static_cast<int>(c.direction) << ':' << c.parameter;
    return out.str();
}

TEST_CASE("runtime-dependent expressions remain explicit and SymbolId based") {
    const char* path = "dynamic_expression_audit.embx";
    { std::ofstream f(path); f << R"(struct Packet {
  count: u8;
  data: bytes[count + 1];
})"; }
    std::string err;
    auto ast = embx::parser::parseFile(path, err); REQUIRE(ast); REQUIRE(err.empty());
    auto ir = embx::ir::lower(*ast, err); REQUIRE(ir); REQUIRE(err.empty());
    auto plan = embx::plan::build(*ir, err); REQUIRE(plan); REQUIRE(err.empty());

    auto sid = plan->symbolTable.findId("Packet");
    auto& s = plan->structs.at(plan->structIndexBySymbol.at(sid));
    auto* data = dynamic_cast<embx::plan::Field*>(s.members.at(1).get());
    REQUIRE(data);
    REQUIRE_FALSE(data->staticLength.has_value());
    REQUIRE(data->type.dimensions.size() == 1);
    auto* e = data->type.dimensions[0].expression.get();
    REQUIRE(e);
    REQUIRE(e->kind == embx::core::ExprKind::Binary);
    REQUIRE(e->left);
    REQUIRE(e->left->reference.valid());
    auto* count = dynamic_cast<embx::plan::Field*>(s.members.at(0).get());
    REQUIRE(count);
    REQUIRE(e->left->reference.id == count->symbol);
    REQUIRE(e->right);
    REQUIRE(e->right->kind == embx::core::ExprKind::Literal);

    e->left->text = "deliberately_invalid_spelling";
    embx::value::Value::Object value;
    value["count"] = uint64_t(2);
    value["data"] = std::vector<uint8_t>{7, 8, 9};
    embx::encoder::Engine enc(*plan);
    auto encoded = enc.encode("Packet", value);
    REQUIRE(encoded.success);
    REQUIRE(encoded.data == std::vector<uint8_t>({2, 7, 8, 9}));
    embx::decoder::Engine dec(*plan);
    auto decoded = dec.decode("Packet", encoded.data);
    REQUIRE(decoded.success);
    REQUIRE(decoded.consumed == encoded.data.size());
    std::remove(path);
}

TEST_CASE("complete executable Plan fingerprint is deterministic") {
    const char* path = "plan_full_determinism.embx";
    { std::ofstream f(path); f << R"(const A = 2;
const B = A + 3;
enum Kind : u8 { zero = 0, five = B }
type Data = bytes[B];
struct Packet {
  first: Data;
  count: u8;
  second: bytes[count];
  variant body by count { 0: u8; default: { x: u16; } }
})"; }

    std::vector<std::string> fingerprints;
    for (int i = 0; i < 5; ++i) {
        std::string err;
        auto ast = embx::parser::parseFile(path, err); REQUIRE(ast); REQUIRE(err.empty());
        auto ir = embx::ir::lower(*ast, err); REQUIRE(ir); REQUIRE(err.empty());
        auto plan = embx::plan::build(*ir, err); REQUIRE(plan); REQUIRE(err.empty());
        fingerprints.push_back(fingerprint(*plan));
    }
    REQUIRE(fingerprints.size() == 5);
    for (std::size_t i = 1; i < fingerprints.size(); ++i) REQUIRE(fingerprints[i] == fingerprints[0]);
    std::remove(path);
}
