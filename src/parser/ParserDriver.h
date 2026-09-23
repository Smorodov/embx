#pragma once
#include "ast/Ast.h"
#include <string>
namespace embx::parser { std::unique_ptr<ast::Module> parseFile(const std::string& path, std::string& error); }
