#pragma once
#include "plan/Plan.h"
#include "runtime/Runtime.h"
#include "value/Value.h"
#include "diagnostics/Diagnostic.h"
#include <cstdint>
#include <string>
#include <vector>

namespace embx::decoder {

using Value = value::Value;

struct Result {
    bool success = false;
    Value value;
    size_t consumed = 0;
    std::string error;
    diagnostics::Diagnostic diagnostic;
    explicit operator bool() const { return success; }
};

struct Options { bool requireFullInput = false; size_t maxArrayElements = 1u << 20; size_t maxAllocationBytes = 64u << 20; size_t maxRecursionDepth = 256; runtime::SymbolEnvironment parameters; };

class Engine {
    const plan::Module& plan_;
    const runtime::CallbackRegistry* callbacks_ = nullptr;
    Options options_;
public:
    explicit Engine(const plan::Module& plan, Options options = {});
    void callbacks(const runtime::CallbackRegistry* registry) { callbacks_ = registry; }
    Result decode(const std::string& structName, const std::vector<uint8_t>& input) const;
};

} // namespace embx::decoder
