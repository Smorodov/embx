#include "compiler/Compiler.h"
#include "parser/ParserDriver.h"
#include "semantic/SemanticAnalyzer.h"
#include "ir/IrBuilder.h"
#include "plan/PlanBuilder.h"
namespace embx::compiler {
std::unique_ptr<Compilation> compileFile(const std::string& path, std::string& error) {
    error.clear();
    auto ast = parser::parseFile(path, error); if (!ast) return {};
    if (!semantic::analyze(*ast, error)) return {};
    auto ir = ir::lower(*ast, error); if (!ir) return {};
    auto plan = plan::build(*ir, error); if (!plan) return {};
    auto result = std::make_unique<Compilation>();
    result->ast = std::move(ast); result->ir = std::move(ir); result->plan = std::move(plan);
    return result;
}
bool generateCppFile(const std::string& path, codegen::Output& output, std::string& error) {
    auto c = compileFile(path, error); if (!c) return false;
    return codegen::generateCpp(*c->plan, output, error);
}
}
