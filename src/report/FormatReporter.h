#pragma once
#include "plan/Plan.h"
#include <string>
namespace embx::report { bool generate(const plan::Module& plan, std::string& output, std::string& error); }
