#pragma once
#include "ir/Ir.h"
#include "plan/Plan.h"
#include <memory>
#include <string>
namespace embx::plan { std::unique_ptr<Module> build(const ir::Module&, std::string& error); }
