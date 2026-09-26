#pragma once
#include "ast/Ast.h"
#include <string>

namespace embx::codegen {

// Reconstructs deterministic canonical EmbX source from the parser AST.
// The generator performs no semantic analysis, name resolution or layout work.
bool generateSource(const ast::Module& module, std::string& output, std::string& error);

} // namespace embx::codegen
