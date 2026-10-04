#pragma once

#include "plan/Plan.h"
#include "runtime/Array.h"

namespace embx::plan {

// Materializes the runtime description of an array from canonical Plan type
// information. The descriptor is metadata only; it does not own storage.
bool makeArrayDescriptor(const Module& plan,
                         const Type& type,
                         const runtime::SymbolEnvironment& environment,
                         runtime::ArrayDescriptor& out,
                         std::string& error);

inline bool makeArrayDescriptor(const Module& plan,
                                const Type& type,
                                runtime::ArrayDescriptor& out,
                                std::string& error) {
    return makeArrayDescriptor(plan, type, runtime::SymbolEnvironment{}, out, error);
}

} // namespace embx::plan
