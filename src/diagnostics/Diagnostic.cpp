#include "diagnostics/Diagnostic.h"
#include <algorithm>
#include <sstream>

namespace embx::diagnostics {

const char* codeName(Code c) noexcept {
    switch(c){
    case Code::None:return "none"; case Code::UnknownStruct:return "unknown_struct";
    case Code::UnknownType:return "unknown_type"; case Code::MissingField:return "missing_field";
    case Code::InvalidValue:return "invalid_value"; case Code::OutOfRange:return "out_of_range";
    case Code::InputTruncated:return "input_truncated"; case Code::OutputConstraint:return "output_constraint";
    case Code::VariantMismatch:return "variant_mismatch"; case Code::BlockConstraint:return "block_constraint";
    case Code::ExpressionError:return "expression_error";
    default:return "internal";
    }
}

static bool has(const std::string& s, const char* x) { return s.find(x) != std::string::npos; }

Code classify(const std::string& m, bool encoding) noexcept {
    if (has(m,"unknown struct")) return Code::UnknownStruct;
    if (has(m,"unknown named type") || has(m,"unsupported scalar type") || has(m,"unknown type")) return Code::UnknownType;
    if (has(m,"missing field") || has(m,"missing variant")) return Code::MissingField;
    if (has(m,"callback") && (has(m,"not configured") || has(m,"direction is not valid"))) return encoding ? Code::OutputConstraint : Code::InvalidValue;
    if (has(m,"no variant case") || has(m,"variant")) return Code::VariantMismatch;
    if (has(m,"block") && (has(m,"size") || has(m,"fill") || has(m,"exceed"))) return Code::BlockConstraint;
    if (has(m,"expression") || has(m,"negative size") || has(m,"size overflow")) return Code::ExpressionError;
    if (!encoding && (has(m,"beyond reader limit") || has(m,"beyond input"))) return Code::InputTruncated;
    if (has(m,"out of range") || has(m,"exceeds") || has(m,"overflow") || has(m,"length mismatch") || has(m,"expected "))
        return encoding ? Code::OutOfRange : Code::InvalidValue;
    return encoding ? Code::OutputConstraint : Code::InvalidValue;
}

Diagnostic make(Code code, std::string message, std::string path, size_t byteOffset,
                size_t expected, size_t actual, bool hasSizeDetail) {
    Diagnostic d; d.code=code; d.message=std::move(message); d.path=std::move(path);
    d.byteOffset=byteOffset; d.expected=expected; d.actual=actual; d.hasSizeDetail=hasSizeDetail; return d;
}

std::string format(const Diagnostic& d){
    std::ostringstream o; o<<codeName(d.code)<<": "; if(!d.path.empty())o<<d.path<<": ";
    o<<d.message; if(d.hasSizeDetail)o<<" [expected="<<d.expected<<", actual="<<d.actual<<"]";
    o<<" @byte "<<d.byteOffset; return o.str();
}
}
