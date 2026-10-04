#pragma once
#include "ast/Ast.h"
#include "ir/Ir.h"
#include <string>
#include <memory>
namespace embx::ir { std::unique_ptr<Module> lower(const ast::Module&, std::string& error); }
