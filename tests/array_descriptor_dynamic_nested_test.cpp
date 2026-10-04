#include "plan/ArrayDescriptor.h"
#include "plan/Plan.h"
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

static std::unique_ptr<embx::core::Expr> literal(const char* text) {
    auto e = std::make_unique<embx::core::Expr>();
    e->kind = embx::core::ExprKind::Literal;
    e->text = text;
    e->type = embx::core::ExprType::IntegerSigned;
    return e;
}

static std::unique_ptr<embx::core::Expr> identifier(const char* text,
                                                    embx::core::SymbolId id) {
    auto e = std::make_unique<embx::core::Expr>();
    e->kind = embx::core::ExprKind::Identifier;
    e->text = text;
    e->type = embx::core::ExprType::IntegerUnsigned;
    e->reference.id = id;
    return e;
}

int main() {
    using namespace embx;
    plan::Module p;

    // Inner is fixed-size and Outer contains Inner.  The array descriptor must
    // use the complete encoded size of Outer without introducing a new layout
    // mechanism.
    plan::Struct inner;
    inner.symbol = 100;
    inner.name = "Inner";
    {
        auto f = std::make_unique<plan::Field>();
        f->name = "a";
        f->type.kind = core::TypeKind::Primitive;
        f->type.name = "u16";
        inner.members.push_back(std::move(f));
    }
    p.structIndexBySymbol.emplace(inner.symbol, p.structs.size());
    p.structs.push_back(std::move(inner));

    plan::Struct outer;
    outer.symbol = 101;
    outer.name = "Outer";
    {
        auto f = std::make_unique<plan::Field>();
        f->name = "inner";
        f->type.kind = core::TypeKind::Named;
        f->type.name = "Inner";
        f->type.reference.id = 100;
        outer.members.push_back(std::move(f));
    }
    {
        auto f = std::make_unique<plan::Field>();
        f->name = "b";
        f->type.kind = core::TypeKind::Primitive;
        f->type.name = "u32";
        outer.members.push_back(std::move(f));
    }
    p.structIndexBySymbol.emplace(outer.symbol, p.structs.size());
    p.structs.push_back(std::move(outer));

    core::Type array;
    array.kind = core::TypeKind::Named;
    array.name = "Outer";
    array.reference.id = 101;
    array.dimensions.push_back(core::Dimension::fixed(literal("2")));
    array.dimensions.push_back(core::Dimension::fixed(literal("3")));

    runtime::ArrayDescriptor descriptor;
    std::string error;
    assert(plan::makeArrayDescriptor(p, array, descriptor, error));
    assert(descriptor.elementKind == runtime::ArrayElementKind::Named);
    assert(descriptor.elementTypeId == 101);
    assert(descriptor.elementSize == 6);
    assert(descriptor.elementCount == 6);
    assert((descriptor.dimensions == std::vector<std::uint64_t>{2, 3}));

    // A dynamic dimension is evaluated through the same runtime expression
    // evaluator and SymbolId environment used elsewhere in execution.
    core::Type dynamic = array;
    dynamic.dimensions.clear();
    dynamic.dimensions.push_back(core::Dimension::dynamic(identifier("count", 900)));
    dynamic.dimensions.push_back(core::Dimension::fixed(literal("3")));

    runtime::SymbolEnvironment environment;
    environment.emplace(900, std::uint64_t{4});
    assert(plan::makeArrayDescriptor(p, dynamic, environment, descriptor, error));
    assert(descriptor.elementCount == 12);
    assert((descriptor.dimensions == std::vector<std::uint64_t>{4, 3}));

    environment[900] = std::uint64_t{0};
    assert(plan::makeArrayDescriptor(p, dynamic, environment, descriptor, error));
    assert(descriptor.elementCount == 0);
    assert((descriptor.dimensions == std::vector<std::uint64_t>{0, 3}));

    return 0;
}
