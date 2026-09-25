#include "plan/ArrayDescriptor.h"

#include <limits>

namespace embx::plan {
namespace {

runtime::ArrayElementKind elementKind(const Type& type) {
    switch (type.kind) {
    case core::TypeKind::Primitive: return runtime::ArrayElementKind::Primitive;
    case core::TypeKind::Bytes: return runtime::ArrayElementKind::Bytes;
    case core::TypeKind::String: return runtime::ArrayElementKind::String;
    case core::TypeKind::Named: return runtime::ArrayElementKind::Named;
    }
    return runtime::ArrayElementKind::Unknown;
}

bool elementSize(const Module& plan, const Type& arrayType,
                 runtime::LayoutSize& out, std::string& error) {
    Type element = arrayType;
    element.dimensions.clear();
    const auto size = fixedTypeSize(plan, element, error);
    if (!size) return false;
    out = *size;
    return true;
}

bool dimensionValue(const core::Dimension& dimension,
                    const runtime::SymbolEnvironment& environment,
                    runtime::LayoutSize& out, std::string& error) {
    if (dimension.kind == core::Dimension::Kind::Remaining) {
        error = "remaining array dimension requires a terminated-sequence runtime boundary";
        return false;
    }
    if (!dimension.expression) {
        error = "array dimension has no expression";
        return false;
    }
    if (!runtime::evaluateSize(dimension.expression.get(), environment, out, error)) {
        return false;
    }
    return true;
}

} // namespace

bool makeArrayDescriptor(const Module& plan,
                         const Type& type,
                         const runtime::SymbolEnvironment& environment,
                         runtime::ArrayDescriptor& out,
                         std::string& error) {
    if (type.dimensions.empty()) {
        error = "array descriptor requires at least one dimension";
        return false;
    }

    runtime::ArrayDescriptor descriptor;
    descriptor.elementKind = elementKind(type);
    if (descriptor.elementKind == runtime::ArrayElementKind::Unknown) {
        error = "unsupported array element type";
        return false;
    }
    descriptor.elementTypeId = type.reference.valid() ? type.reference.id : 0;

    if (!elementSize(plan, type, descriptor.elementSize, error)) {
        if (error.empty()) error = "array element type has no fixed encoded size";
        return false;
    }

    descriptor.dimensions.reserve(type.dimensions.size());
    for (const auto& dimension : type.dimensions) {
        runtime::LayoutSize extent = 0;
        if (!dimensionValue(dimension, environment, extent, error)) return false;
        descriptor.dimensions.push_back(extent);
    }

    if (!runtime::checkedElementCount(descriptor.dimensions,
                                      descriptor.elementCount, error)) {
        return false;
    }

    if (!descriptor.valid(&error)) return false;
    out = std::move(descriptor);
    return true;
}

} // namespace embx::plan
