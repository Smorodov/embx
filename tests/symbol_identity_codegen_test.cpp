#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "codegen/CppGenerator.h"
#include "plan/Plan.h"

TEST_CASE("C++ generator resolves by-value dependencies by SymbolId") {
    embx::plan::Module p;
    const auto a = p.symbolTable.declare(embx::core::SymbolKind::Struct, "A");
    const auto b = p.symbolTable.declare(embx::core::SymbolKind::Struct, "B");
    embx::plan::Struct B; B.symbol = b; B.name = "B";
    embx::plan::Struct A; A.symbol = a; A.name = "A";
    auto f = std::make_unique<embx::plan::Field>();
    f->name = "b"; f->type.kind = embx::core::TypeKind::Named; f->type.name = "B"; f->type.reference.id = b;
    A.members.push_back(std::move(f));
    p.structs.push_back(std::move(B));
    p.structs.push_back(std::move(A));
    p.structIndexBySymbol[b] = 0; p.structIndexBySymbol[a] = 1;

    std::string err; embx::codegen::Output out;
    REQUIRE(embx::codegen::generateCpp(p, out, err));
    REQUIRE(err.empty());
}
