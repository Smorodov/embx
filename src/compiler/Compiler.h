#pragma once
#include "ast/Ast.h"
#include "ir/Ir.h"
#include "plan/Plan.h"
#include "codegen/CppGenerator.h"
#include <memory>
#include <string>
namespace embx::compiler {
struct Compilation { std::unique_ptr<ast::Module> ast; std::unique_ptr<ir::Module> ir; std::unique_ptr<plan::Module> plan; };
std::unique_ptr<Compilation> compileFile(const std::string& path, std::string& error);
bool generateCppFile(const std::string& path, codegen::Output& output, std::string& error);
}
