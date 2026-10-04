#include "TestPlan.h"
#include <catch2/catch_test_macros.hpp>
#include "codegen/CppGenerator.h"
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"

static embx::codegen::Output generateContainer() {
    std::string error;
    auto ast = embx::parser::parseFile("../examples/container.embx", error);
    REQUIRE(ast);
    REQUIRE(error.empty());
    REQUIRE(embx::semantic::analyze(*ast, error));
    REQUIRE(error.empty());
    auto ir = embx::ir::lower(*ast, error);
    REQUIRE(ir);
    REQUIRE(error.empty());
    auto plan = embx::plan::build(*ir, error);
    REQUIRE(plan);
    REQUIRE(error.empty());
    embx::codegen::Output out;
    const bool generated = embx::codegen::generateCpp(*plan, out, error);
    INFO("generateCpp error: " << error);
    REQUIRE(generated);
    REQUIRE(error.empty());
    return out;
}

TEST_CASE("generated dynamic layout uses checked vectors and bounded blocks") {
    const auto out = generateContainer();
    REQUIRE(out.header.find("std::vector<std::uint8_t> data") != std::string::npos);
    REQUIRE(out.header.find("struct Container_payload") != std::string::npos);
    REQUIRE(out.header.find("Container_payload payload") != std::string::npos);
    REQUIRE(out.source.find("Reader br(r.d,r.p,blockN)") != std::string::npos);
    REQUIRE(out.source.find("checkedHostSize(blockN64,blockN)") != std::string::npos);
    REQUIRE(out.source.find("r.getBytes(out.data,r.remaining())") != std::string::npos);
    REQUIRE(out.source.find("block size mismatch") != std::string::npos);
    REQUIRE(out.source.find("align64=") != std::string::npos);
    REQUIRE(out.source.find("block size mismatch") != std::string::npos);
}

TEST_CASE("generated dynamic layout checks declared array count") {
    std::string error;
    embx::plan::Module module;
    embx::plan::Struct s; s.name = "Dynamic";
    auto count = std::make_unique<embx::plan::Field>();
    count->name="count"; count->type.kind=embx::core::TypeKind::Primitive; count->type.name="u8";
    const auto countId = embx::test::addFieldSymbol(module, *count);
    s.members.push_back(std::move(count));
    auto values = std::make_unique<embx::plan::Field>();
    values->name="values"; values->type.kind=embx::core::TypeKind::Primitive; values->type.name="u16";
    auto ex=std::make_unique<embx::plan::Expr>(); ex->kind=embx::core::ExprKind::Identifier; ex->text="count"; ex->reference.id=countId;
    values->type.dimensions.push_back(embx::core::Dimension::dynamic(std::move(ex)));
    s.members.push_back(std::move(values));
    embx::test::addStruct(module, std::move(s));
    embx::codegen::Output out;
    const bool generated = embx::codegen::generateCpp(module,out,error);
    INFO("generateCpp error: " << error);
    REQUIRE(generated);
    REQUIRE(error.empty());
    REQUIRE(out.header.find("std::vector<std::uint16_t> values") != std::string::npos);
    const bool hasCheckedCount =
        out.source.find("n064=embx_generated_detail::checkedSize(out.count)") != std::string::npos ||
        out.source.find("n64=embx_generated_detail::checkedSize(out.count)") != std::string::npos;
    REQUIRE(hasCheckedCount);
    const bool hasElementLimit =
        out.source.find("n064>1048576ULL") != std::string::npos ||
        out.source.find("n64>1048576ULL") != std::string::npos;
    REQUIRE(hasElementLimit);
}


TEST_CASE("generated block type permits alignment operations") {
    std::string error;
    embx::plan::Module module;
    embx::plan::Struct s; s.name = "Aligned";
    auto block = std::make_unique<embx::plan::Block>();
    block->name = "payload";
    block->staticSize = 4;

    auto first = std::make_unique<embx::plan::Field>();
    first->name = "x";
    first->type.kind = embx::core::TypeKind::Primitive;
    first->type.name = "u8";
    block->members.push_back(std::move(first));

    auto align = std::make_unique<embx::plan::Align>();
    align->staticAlignment = 2;
    align->alignment = std::make_unique<embx::plan::Expr>();
    align->alignment->kind = embx::core::ExprKind::Literal;
    align->alignment->text = "2";
    block->members.push_back(std::move(align));

    auto second = std::make_unique<embx::plan::Field>();
    second->name = "y";
    second->type.kind = embx::core::TypeKind::Primitive;
    second->type.name = "u16";
    block->members.push_back(std::move(second));

    s.members.push_back(std::move(block));
    
    embx::test::addStruct(module, std::move(s));

    embx::codegen::Output out;
    REQUIRE(embx::codegen::generateCpp(module, out, error));
    REQUIRE(error.empty());
    REQUIRE(out.header.find("struct Aligned_payload") != std::string::npos);
    REQUIRE(out.header.find("std::uint8_t x") != std::string::npos);
    REQUIRE(out.header.find("std::uint16_t y") != std::string::npos);
    REQUIRE(out.source.find("align64=") != std::string::npos);
}


TEST_CASE("generated layout uses static at and alignment values") {
    std::string error;
    embx::plan::Module module;
    embx::plan::Struct s; s.name = "StaticLayout";
    auto x = std::make_unique<embx::plan::Field>();
    x->name = "x"; x->type.kind = embx::core::TypeKind::Primitive; x->type.name = "u8";
    s.members.push_back(std::move(x));
    auto at = std::make_unique<embx::plan::At>();
    at->staticOffset = 4;
    auto y = std::make_unique<embx::plan::Field>();
    y->name = "y"; y->type.kind = embx::core::TypeKind::Primitive; y->type.name = "u16";
    at->members.push_back(std::move(y));
    s.members.push_back(std::move(at));
    auto align = std::make_unique<embx::plan::Align>();
    align->staticAlignment = 8;
    s.members.push_back(std::move(align));
    
    embx::test::addStruct(module, std::move(s));
    embx::codegen::Output out;
    REQUIRE(embx::codegen::generateCpp(module, out, error));
    REQUIRE(error.empty());
    REQUIRE(out.source.find("static_cast<std::uint64_t>(4ULL)") != std::string::npos);
    REQUIRE(out.source.find("static_cast<std::uint64_t>(8ULL)") != std::string::npos);
}

TEST_CASE("generated block type supports bits operations") {
    std::string error;
    embx::plan::Module module;
    embx::plan::Struct s; s.name = "BlockBits";
    auto block = std::make_unique<embx::plan::Block>();
    block->name = "payload";
    block->staticSize = 1;
    auto bits = std::make_unique<embx::plan::Bits>();
    bits->storageBytes = 1;
    bits->totalBits = 8;
    embx::plan::BitsField lo; lo.name = "lo"; lo.type.kind = embx::core::TypeKind::Primitive; lo.type.name = "u8"; lo.bits = 4;
    embx::plan::BitsField hi; hi.name = "hi"; hi.type.kind = embx::core::TypeKind::Primitive; hi.type.name = "u8"; hi.bits = 4;
    bits->fields.push_back(std::move(lo));
    bits->fields.push_back(std::move(hi));
    block->members.push_back(std::move(bits));
    s.members.push_back(std::move(block));
    
    embx::test::addStruct(module, std::move(s));

    embx::codegen::Output out;
    REQUIRE(embx::codegen::generateCpp(module, out, error));
    REQUIRE(error.empty());
    REQUIRE(out.header.find("struct BlockBits_payload") != std::string::npos);
    REQUIRE(out.header.find("std::uint8_t lo") != std::string::npos);
    REQUIRE(out.header.find("std::uint8_t hi") != std::string::npos);
    REQUIRE(out.source.find("bitsRaw") != std::string::npos);
}

TEST_CASE("generated at layout permits alignment operations") {
    std::string error;
    embx::plan::Module module;
    embx::plan::Struct s; s.name = "AtAligned";
    auto at = std::make_unique<embx::plan::At>(); at->staticOffset = 2;
    auto x = std::make_unique<embx::plan::Field>(); x->name="x"; x->type.kind=embx::core::TypeKind::Primitive; x->type.name="u8";
    at->members.push_back(std::move(x));
    auto align = std::make_unique<embx::plan::Align>(); align->staticAlignment=2;
    align->alignment=std::make_unique<embx::plan::Expr>(); align->alignment->kind=embx::core::ExprKind::Literal; align->alignment->text="2";
    at->members.push_back(std::move(align));
    auto y = std::make_unique<embx::plan::Field>(); y->name="y"; y->type.kind=embx::core::TypeKind::Primitive; y->type.name="u16";
    at->members.push_back(std::move(y));
    s.members.push_back(std::move(at)); embx::test::addStruct(module, std::move(s));
    embx::codegen::Output out; REQUIRE(embx::codegen::generateCpp(module,out,error)); REQUIRE(error.empty());
    REQUIRE(out.source.find("align64=") != std::string::npos);
}

TEST_CASE("generated codec rejects plans without SymbolId") {
    std::string error;
    embx::plan::Module module;
    embx::plan::Struct s;
    s.name = "IdentityFree";
    auto x = std::make_unique<embx::plan::Field>();
    x->name = "x";
    x->type.kind = embx::core::TypeKind::Primitive;
    x->type.name = "u8";
    s.members.push_back(std::move(x));
    module.structs.push_back(std::move(s));

    embx::codegen::Output out;
    REQUIRE_FALSE(embx::codegen::generateCpp(module, out, error));
    REQUIRE(error.find("SymbolId") != std::string::npos);
}

TEST_CASE("generated codec supports conditional fields") {
    std::string error;
    embx::plan::Module module;
    embx::plan::Struct s; s.name = "ConditionalGenerated";

    auto flag = std::make_unique<embx::plan::Field>();
    flag->name = "flag";
    flag->type.kind = embx::core::TypeKind::Primitive;
    flag->type.name = "u8";
    const auto flagId = embx::test::addFieldSymbol(module, *flag);
    s.members.push_back(std::move(flag));

    auto cond = std::make_unique<embx::plan::Conditional>();
    cond->condition = std::make_unique<embx::plan::Expr>();
    cond->condition->kind = embx::core::ExprKind::Identifier;
    cond->condition->text = "flag";
    cond->condition->reference.id = flagId;

    auto yes = std::make_unique<embx::plan::Field>();
    yes->name = "when_set";
    yes->type.kind = embx::core::TypeKind::Primitive;
    yes->type.name = "u16";
    cond->thenMembers.push_back(std::move(yes));

    auto no = std::make_unique<embx::plan::Field>();
    no->name = "when_clear";
    no->type.kind = embx::core::TypeKind::Primitive;
    no->type.name = "u32";
    cond->elseMembers.push_back(std::move(no));

    s.members.push_back(std::move(cond));
    embx::test::addStruct(module, std::move(s));

    embx::codegen::Output out;
    REQUIRE(embx::codegen::generateCpp(module, out, error));
    REQUIRE(error.empty());
    REQUIRE(out.header.find("std::uint16_t when_set{};") != std::string::npos);
    REQUIRE(out.header.find("std::uint32_t when_clear{};") != std::string::npos);
    REQUIRE(out.source.find("if(out.flag){") != std::string::npos);
    const bool hasFlagCondition =
        out.source.find("if(value.flag){") != std::string::npos ||
        out.source.find("if(work.flag){") != std::string::npos;
    REQUIRE(hasFlagCondition);
}


TEST_CASE("generated expression boundary carries runtime parameters") {
    embx::plan::Module module;

    embx::plan::Parameter parameter;
    parameter.name = "count";
    parameter.type.kind = embx::core::TypeKind::Primitive;
    parameter.type.name = "u8";
    parameter.symbol = module.symbolTable.declare(embx::core::SymbolKind::Parameter, parameter.name);
    REQUIRE(parameter.symbol != embx::core::InvalidSymbolId);
    module.parameterIndexBySymbol[parameter.symbol] = module.parameters.size();
    module.parameters.push_back(parameter);

    embx::plan::Struct s;
    s.name = "RuntimeSized";

    auto values = std::make_unique<embx::plan::Field>();
    values->name = "values";
    values->type.kind = embx::core::TypeKind::Primitive;
    values->type.name = "u16";
    auto length = std::make_unique<embx::plan::Expr>();
    length->kind = embx::core::ExprKind::Identifier;
    length->text = "count";
    length->reference.id = parameter.symbol;
    length->type = embx::core::ExprType::IntegerUnsigned;
    values->type.dimensions.push_back(embx::core::Dimension::dynamic(std::move(length)));
    s.members.push_back(std::move(values));

    embx::test::addStruct(module, std::move(s));

    std::string error;
    embx::codegen::Output out;
    REQUIRE(embx::codegen::generateCpp(module, out, error));
    REQUIRE(error.empty());
    REQUIRE(out.header.find("struct RuntimeParameters") != std::string::npos);
    REQUIRE(out.header.find("std::uint8_t count{};") != std::string::npos);
    REQUIRE(out.header.find("const embx_generated_detail::RuntimeParameters& embx_generated_params") != std::string::npos);
    REQUIRE(out.source.find("checkedSize(embx_generated_params.count)") != std::string::npos);
}

TEST_CASE("generated codec preserves multidimensional fixed array shape") {
    std::string error;
    embx::plan::Module module;
    embx::plan::Struct s; s.name = "Matrix";
    auto values = std::make_unique<embx::plan::Field>();
    values->name = "values";
    values->type.kind = embx::core::TypeKind::Primitive;
    values->type.name = "u16";
    auto d0 = std::make_unique<embx::plan::Expr>();
    d0->kind = embx::core::ExprKind::Literal; d0->type = embx::core::ExprType::IntegerUnsigned; d0->text = "2";
    auto d1 = std::make_unique<embx::plan::Expr>();
    d1->kind = embx::core::ExprKind::Literal; d1->type = embx::core::ExprType::IntegerUnsigned; d1->text = "3";
    values->type.dimensions.push_back(embx::core::Dimension::fixed(std::move(d0)));
    values->type.dimensions.push_back(embx::core::Dimension::fixed(std::move(d1)));
    s.members.push_back(std::move(values));
    embx::test::addStruct(module, std::move(s));

    embx::codegen::Output out;
    REQUIRE(embx::codegen::generateCpp(module, out, error));
    REQUIRE(error.empty());
    REQUIRE(out.header.find("std::array<std::array<std::uint16_t, 3>, 2> values") != std::string::npos);
    REQUIRE(out.source.find("i0<2ULL") != std::string::npos);
    REQUIRE(out.source.find("i1<3ULL") != std::string::npos);
}

TEST_CASE("generated codec recursively handles dynamic multidimensional extents") {
    std::string error;
    embx::plan::Module module;
    embx::plan::Struct s; s.name = "DynamicMatrix";

    auto count = std::make_unique<embx::plan::Field>();
    count->name = "count"; count->type.kind = embx::core::TypeKind::Primitive; count->type.name = "u8";
    const auto countId = embx::test::addFieldSymbol(module, *count);
    s.members.push_back(std::move(count));

    auto values = std::make_unique<embx::plan::Field>();
    values->name = "values"; values->type.kind = embx::core::TypeKind::Primitive; values->type.name = "u16";
    auto d0 = std::make_unique<embx::plan::Expr>();
    d0->kind = embx::core::ExprKind::Identifier; d0->text = "count"; d0->reference.id = countId;
    d0->type = embx::core::ExprType::IntegerUnsigned;
    auto d1 = std::make_unique<embx::plan::Expr>();
    d1->kind = embx::core::ExprKind::Literal; d1->type = embx::core::ExprType::IntegerUnsigned; d1->text = "3";
    values->type.dimensions.push_back(embx::core::Dimension::dynamic(std::move(d0)));
    values->type.dimensions.push_back(embx::core::Dimension::fixed(std::move(d1)));
    s.members.push_back(std::move(values));
    embx::test::addStruct(module, std::move(s));

    embx::codegen::Output out;
    REQUIRE(embx::codegen::generateCpp(module, out, error));
    REQUIRE(error.empty());
    REQUIRE(out.header.find("std::array<std::uint16_t, 3>") != std::string::npos);
    REQUIRE(out.header.find("std::vector<std::array<std::uint16_t, 3>> values") != std::string::npos);
    REQUIRE(out.source.find("n064=embx_generated_detail::checkedSize(out.count)") != std::string::npos);
    REQUIRE(out.source.find("i0<n0") != std::string::npos);
    REQUIRE(out.source.find("i1<3ULL") != std::string::npos);
}
