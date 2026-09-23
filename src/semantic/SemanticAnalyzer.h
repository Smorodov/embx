#pragma once
#include "ast/Ast.h"
#include <string>
namespace embx::semantic { bool analyze(const ast::Module&, std::string& error); }
