#pragma once
#include "diagnostics/Diagnostic.h"
#include "value/Value.h"
#include "plan/Plan.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "runtime/Runtime.h"

namespace embx::encoder {
using Value = value::Value;
struct Result { bool success=false; std::vector<uint8_t> data; std::string error; diagnostics::Diagnostic diagnostic; explicit operator bool() const { return success; } };
using CallbackValue = runtime::Value;
using CallbackArgs = std::vector<CallbackValue>;
struct Options { size_t maxArrayElements=1u<<20; size_t maxAllocationBytes=64u<<20; size_t maxRecursionDepth=256; bool allowTrailingBlockBytes=true; const runtime::CallbackRegistry* callbacks=nullptr; runtime::SymbolEnvironment parameters; };
class Engine {
    const plan::Module& plan_; Options options_;
public:
    explicit Engine(const plan::Module& plan, Options options={});
    Result encode(const std::string& structName, const Value& value) const;
};
} // namespace embx::encoder
