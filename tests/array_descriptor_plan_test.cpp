#include "plan/ArrayDescriptor.h"
#include <cassert>
#include <cstdint>
#include <memory>

static std::unique_ptr<embx::core::Expr> literal(const char* text) {
    auto e = std::make_unique<embx::core::Expr>();
    e->kind = embx::core::ExprKind::Literal;
    e->text = text;
    e->type = embx::core::ExprType::IntegerSigned;
    return e;
}


using embx::core::Dimension;
using embx::core::Type;
using embx::core::TypeKind;
using embx::runtime::ArrayElementKind;

int main() {
    embx::plan::Module plan;

    Type scalar;
    scalar.kind = TypeKind::Primitive;
    scalar.name = "u32";

    Type primitiveArray = scalar;
    primitiveArray.dimensions.push_back(Dimension::fixed(literal("2")));
    primitiveArray.dimensions.push_back(Dimension::fixed(literal("3")));

    embx::runtime::ArrayDescriptor descriptor;
    std::string error;
    assert(embx::plan::makeArrayDescriptor(plan, primitiveArray, descriptor, error));
    assert(descriptor.elementKind == ArrayElementKind::Primitive);
    assert(descriptor.elementSize == 4);
    assert(descriptor.elementCount == 6);
    assert((descriptor.dimensions == std::vector<std::uint64_t>{2, 3}));

    embx::plan::Struct packet;
    packet.symbol = 42;
    packet.name = "Packet";
    auto field = std::make_unique<embx::plan::Field>();
    field->name = "value";
    field->type = scalar;
    packet.members.push_back(std::move(field));
    plan.structIndexBySymbol.emplace(packet.symbol, plan.structs.size());
    plan.structs.push_back(std::move(packet));

    Type packetArray;
    packetArray.kind = TypeKind::Named;
    packetArray.name = "Packet";
    packetArray.reference.id = 42;
    packetArray.dimensions.push_back(Dimension::fixed(literal("2")));
    packetArray.dimensions.push_back(Dimension::fixed(literal("3")));

    assert(embx::plan::makeArrayDescriptor(plan, packetArray, descriptor, error));
    assert(descriptor.elementKind == ArrayElementKind::Named);
    assert(descriptor.elementTypeId == 42);
    assert(descriptor.elementSize == 4);
    assert(descriptor.elementCount == 6);

    return 0;
}
