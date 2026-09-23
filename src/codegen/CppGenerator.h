#pragma once
#include "plan/Plan.h"
#include <string>

namespace embx::codegen {

struct Output {
    std::string header;
    std::string source;
};

// Generates a C++17 type and binary-codec representation from the validated Plan.
// Generates the validated EmbX layout subset with checked logical 64-bit layout quantities;
// constructs outside the generated backend are rejected.
bool generateCpp(const plan::Module& module, Output& output, std::string& error);

} // namespace embx::codegen
