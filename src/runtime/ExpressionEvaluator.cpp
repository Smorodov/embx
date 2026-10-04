#include "runtime/ExpressionEvaluator.h"
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <sstream>

namespace embx::runtime {
namespace {

long double asReal(const Value& v) {
    if (auto p = std::get_if<int64_t>(&v)) return static_cast<long double>(*p);
    if (auto p = std::get_if<uint64_t>(&v)) return static_cast<long double>(*p);
    if (auto p = std::get_if<double>(&v)) return static_cast<long double>(*p);
    return std::get<bool>(v) ? 1.0L : 0.0L;
}
bool parseLiteral(const std::string& s, Value& out, std::string& err) {
    if (s.empty()) { err = "empty literal"; return false; }
    if (s == "true") { out = true; return true; }
    if (s == "false") { out = false; return true; }
    if (s.front() == '"') { err = "string literal is not numeric"; return false; }
    if (s.find('.') != std::string::npos) {
        char* end = nullptr; errno = 0;
        const double v = std::strtod(s.c_str(), &end);
        if (errno == ERANGE || !end || *end) { err = "invalid floating literal: " + s; return false; }
        out = v; return true;
    }
    try {
        size_t idx = 0;
        if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            const auto v = std::stoull(s, &idx, 16);
            if (idx != s.size()) throw std::invalid_argument("trailing");
            out = static_cast<uint64_t>(v); return true;
        }
        const auto v = std::stoull(s, &idx, 10);
        if (idx != s.size()) throw std::invalid_argument("trailing");
        if (v <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) out = static_cast<int64_t>(v);
        else out = static_cast<uint64_t>(v);
        return true;
    } catch (...) { err = "invalid integer literal: " + s; return false; }
}

bool checkedSigned(int64_t a, int64_t b, char op, Value& out, std::string& err) {
    if (op == '+') {
        if ((b > 0 && a > std::numeric_limits<int64_t>::max() - b) ||
            (b < 0 && a < std::numeric_limits<int64_t>::min() - b)) { err="integer overflow"; return false; }
        out = a + b; return true;
    }
    if (op == '-') {
        if ((b < 0 && a > std::numeric_limits<int64_t>::max() + b) ||
            (b > 0 && a < std::numeric_limits<int64_t>::min() + b)) { err="integer overflow"; return false; }
        out = a - b; return true;
    }
    if (op == '*') {
        if (a == 0 || b == 0) { out = int64_t(0); return true; }
        if (a == -1) { if (b == std::numeric_limits<int64_t>::min()) { err="integer overflow"; return false; } out = -b; return true; }
        if (b == -1) { if (a == std::numeric_limits<int64_t>::min()) { err="integer overflow"; return false; } out = -a; return true; }
        if (a > 0) {
            if (b > 0) { if (a > std::numeric_limits<int64_t>::max() / b) { err="integer overflow"; return false; } }
            else if (b < std::numeric_limits<int64_t>::min() / a) { err="integer overflow"; return false; }
        } else {
            if (b > 0) { if (a < std::numeric_limits<int64_t>::min() / b) { err="integer overflow"; return false; } }
            else if (a < std::numeric_limits<int64_t>::max() / b) { err="integer overflow"; return false; }
        }
        out = a * b; return true;
    }
    return false;
}
bool integerOperands(const Value& a, const Value& b, uint64_t& ua, uint64_t& ub, bool& unsignedMode, std::string& err) {
    const bool au = std::holds_alternative<uint64_t>(a);
    const bool bu = std::holds_alternative<uint64_t>(b);
    const bool as = std::holds_alternative<int64_t>(a);
    const bool bs = std::holds_alternative<int64_t>(b);
    if (!(au || as) || !(bu || bs)) { err = "operands are not integers"; return false; }
    if (au && bu) { ua=std::get<uint64_t>(a); ub=std::get<uint64_t>(b); unsignedMode=true; return true; }
    if (as && bs) { ua=static_cast<uint64_t>(std::get<int64_t>(a)); ub=static_cast<uint64_t>(std::get<int64_t>(b)); unsignedMode=false; return true; }
    // Deterministic mixed signed/unsigned rule: if the signed value is
    // negative, mixed integer arithmetic/comparison is invalid rather than
    // silently wrapping it into uint64_t. Non-negative signed values are
    // promoted exactly to uint64_t.
    const int64_t sv = as ? std::get<int64_t>(a) : std::get<int64_t>(b);
    if (sv < 0) { err = "mixed signed/unsigned operation with negative signed operand"; return false; }
    ua = as ? static_cast<uint64_t>(sv) : std::get<uint64_t>(a);
    ub = bs ? static_cast<uint64_t>(std::get<int64_t>(b)) : std::get<uint64_t>(b);
    unsignedMode=true;
    return true;
}

int compareIntegers(const Value& a, const Value& b, std::string& err) {
    uint64_t ua=0, ub=0; bool um=false;
    if (!integerOperands(a,b,ua,ub,um,err)) return 2;
    if (um) return ua<ub ? -1 : ua>ub ? 1 : 0;
    const int64_t sa=static_cast<int64_t>(ua), sb=static_cast<int64_t>(ub);
    return sa<sb ? -1 : sa>sb ? 1 : 0;
}

bool binary(const std::string& op, const Value& a, const Value& b, Value& out, std::string& err) {
    if (op == "&&" || op == "||") {
        if (!std::holds_alternative<bool>(a) || !std::holds_alternative<bool>(b)) {
            err = "logical operator requires boolean operands";
            return false;
        }
        const bool x = std::get<bool>(a), y = std::get<bool>(b);
        out = (op == "&&") ? (x && y) : (x || y);
        return true;
    }

    if (op == "==" || op == "!=" || op == "<" || op == "<=" || op == ">" || op == ">=") {
        const bool aNumeric = std::holds_alternative<int64_t>(a) || std::holds_alternative<uint64_t>(a) || std::holds_alternative<double>(a);
        const bool bNumeric = std::holds_alternative<int64_t>(b) || std::holds_alternative<uint64_t>(b) || std::holds_alternative<double>(b);
        if (!aNumeric || !bNumeric) { err = "comparison operands must be numeric"; return false; }
        if (std::holds_alternative<double>(a) || std::holds_alternative<double>(b)) {
            const long double x=asReal(a), y=asReal(b);
            if (op=="==") out=x==y; else if(op=="!=") out=x!=y; else if(op=="<") out=x<y; else if(op=="<=") out=x<=y; else if(op==">") out=x>y; else out=x>=y;
            return true;
        }
        std::string cmpErr; const int c=compareIntegers(a,b,cmpErr);
        if (c==2) { err=cmpErr; return false; }
        if(op=="==") out=c==0; else if(op=="!=") out=c!=0; else if(op=="<") out=c<0; else if(op=="<=") out=c<=0; else if(op==">") out=c>0; else out=c>=0;
        return true;
    }

    if (std::holds_alternative<double>(a) || std::holds_alternative<double>(b)) {
        const long double x=asReal(a), y=asReal(b);
        if ((op=="/" || op=="%") && y==0) { err="division by zero"; return false; }
        long double z=0;
        if(op=="+")z=x+y; else if(op=="-")z=x-y; else if(op=="*")z=x*y; else if(op=="/")z=x/y; else { err="% requires integer operands"; return false; }
        if(!std::isfinite(static_cast<double>(z))) { err="floating arithmetic overflow"; return false; }
        out=static_cast<double>(z); return true;
    }

    uint64_t ua=0, ub=0; bool um=false;
    if (!integerOperands(a,b,ua,ub,um,err)) return false;
    if (!um) {
        const int64_t x=static_cast<int64_t>(ua), y=static_cast<int64_t>(ub);
        if((op=="/"||op=="%")&&y==0){err="division by zero";return false;}
        if(op=="/" && x==std::numeric_limits<int64_t>::min() && y==-1){err="integer overflow";return false;}
        if(op=="%" && x==std::numeric_limits<int64_t>::min() && y==-1){out=int64_t(0);return true;}
        if(op=="+"||op=="-"||op=="*") return checkedSigned(x,y,op[0],out,err);
        if(op=="/"){out=x/y;return true;} if(op=="%"){out=x%y;return true;}
    }
    if((op=="/"||op=="%")&&ub==0){err="division by zero";return false;}
    if(op=="+"){if(ub>std::numeric_limits<uint64_t>::max()-ua){err="integer overflow";return false;}out=ua+ub;return true;}
    if(op=="-"){if(ub>ua){err="unsigned integer underflow";return false;}out=ua-ub;return true;}
    if(op=="*"){if(ub && ua>std::numeric_limits<uint64_t>::max()/ub){err="integer overflow";return false;}out=ua*ub;return true;}
    if(op=="/"){out=ua/ub;return true;} if(op=="%"){out=ua%ub;return true;}
    err="unsupported binary operation"; return false;
}

bool eval(const core::Expr* e, const Environment& env, Value& out, std::string& err) {
    if(!e){err="missing expression";return false;}
    switch(e->kind){
    case core::ExprKind::Literal: return parseLiteral(e->text,out,err);
    case core::ExprKind::Identifier:{auto it=env.find(e->text);if(it==env.end()){err="unknown identifier: "+e->text;return false;}out=it->second;return true;}
    case core::ExprKind::Parenthesized:return eval(e->left.get(),env,out,err);
    case core::ExprKind::Unary:{if(e->op!="-"){err="unknown unary operator: "+e->op;return false;}Value v;if(!eval(e->right.get(),env,v,err))return false;if(auto p=std::get_if<int64_t>(&v)){if(*p==std::numeric_limits<int64_t>::min()){err="integer overflow";return false;}out=-*p;return true;}if(auto p=std::get_if<uint64_t>(&v)){if(*p>static_cast<uint64_t>(std::numeric_limits<int64_t>::max())+1ULL){err="integer overflow";return false;}if(*p==static_cast<uint64_t>(std::numeric_limits<int64_t>::max())+1ULL)out=std::numeric_limits<int64_t>::min();else out=-static_cast<int64_t>(*p);return true;}if(auto p=std::get_if<double>(&v)){out=-*p;return true;}err="cannot negate boolean";return false;}
    case core::ExprKind::Binary:{
        Value a;
        if(!eval(e->left.get(),env,a,err)) return false;
        if(e->op=="&&") {
            if(!std::holds_alternative<bool>(a)) { err="logical operator requires boolean operands"; return false; }
            if(!std::get<bool>(a)) { out=false; return true; }
            Value b; if(!eval(e->right.get(),env,b,err)) return false;
            return binary(e->op,a,b,out,err);
        }
        if(e->op=="||") {
            if(!std::holds_alternative<bool>(a)) { err="logical operator requires boolean operands"; return false; }
            if(std::get<bool>(a)) { out=true; return true; }
            Value b; if(!eval(e->right.get(),env,b,err)) return false;
            return binary(e->op,a,b,out,err);
        }
        Value b;
        if(!eval(e->right.get(),env,b,err)) return false;
        return binary(e->op,a,b,out,err);
    }
    }
    err="unknown expression kind";return false;
}
}

bool evaluate(const core::Expr* expr, const Environment& env, Value& out, std::string& error){return eval(expr,env,out,error);}
bool evalSymbol(const core::Expr* e, const SymbolEnvironment& env, Value& out, std::string& err) {
    if (!e) { err = "missing expression"; return false; }
    if (e->kind == core::ExprKind::Identifier) {
        if (!e->reference.valid()) { err = "unresolved identifier: " + e->text; return false; }
        auto it = env.find(e->reference.id);
        if (it == env.end()) { err = "unknown symbol: " + e->text; return false; }
        out = it->second; return true;
    }
    if (e->kind == core::ExprKind::Literal) return parseLiteral(e->text, out, err);
    if (e->kind == core::ExprKind::Parenthesized) return evalSymbol(e->left.get(), env, out, err);
    if (e->kind == core::ExprKind::Unary) {
        Value v; if (!evalSymbol(e->right.get(), env, v, err)) return false;
        if (e->op != "-") { err = "unknown unary operator: " + e->op; return false; }
        if (auto p=std::get_if<int64_t>(&v)) { if (*p==std::numeric_limits<int64_t>::min()) { err="integer overflow"; return false; } out=-*p; return true; }
        if (auto p=std::get_if<uint64_t>(&v)) { if (*p>uint64_t(std::numeric_limits<int64_t>::max())+1ULL) {err="integer overflow";return false;} out=(*p==uint64_t(std::numeric_limits<int64_t>::max())+1ULL)?Value(std::numeric_limits<int64_t>::min()):-static_cast<int64_t>(*p); return true; }
        if (auto p=std::get_if<double>(&v)) { out=-*p; return true; }
        err="cannot negate boolean"; return false;
    }
    if (e->kind == core::ExprKind::Binary) {
        Value a; if (!evalSymbol(e->left.get(), env, a, err)) return false;
        if (e->op=="&&") {
            if (!std::holds_alternative<bool>(a)) { err="logical operator requires boolean operands"; return false; }
            if (!std::get<bool>(a)) { out=false; return true; }
        }
        if (e->op=="||") {
            if (!std::holds_alternative<bool>(a)) { err="logical operator requires boolean operands"; return false; }
            if (std::get<bool>(a)) { out=true; return true; }
        }
        Value b; if (!evalSymbol(e->right.get(), env, b, err)) return false;
        return binary(e->op,a,b,out,err);
    }
    err="unknown expression kind"; return false;
}

bool evaluate(const core::Expr* expr, const SymbolEnvironment& env, Value& out, std::string& error){return evalSymbol(expr,env,out,error);}


bool evaluateInteger(const core::Expr* expr, const Environment& env, int64_t& out, std::string& error){
    Value v;if(!eval(expr,env,v,error))return false;
    if(auto p=std::get_if<int64_t>(&v)){out=*p;return true;}
    if(auto p=std::get_if<uint64_t>(&v)){if(*p>static_cast<uint64_t>(std::numeric_limits<int64_t>::max())){error="integer value out of signed range";return false;}out=static_cast<int64_t>(*p);return true;}
    error="expression is not an integer";return false;
}

bool evaluateSize(const core::Expr* expr, const Environment& env, LayoutSize& out, std::string& error){
    Value v;if(!eval(expr,env,v,error))return false;
    uint64_t x=0;
    if(auto signedValue=std::get_if<int64_t>(&v)){if(*signedValue<0){error="size/offset cannot be negative";return false;}x=static_cast<uint64_t>(*signedValue);}
    else if(auto unsignedValue=std::get_if<uint64_t>(&v))x=*unsignedValue;
    else {error="size/offset must be an integer";return false;}
    out=x;return true;
}

bool evaluateInteger(const core::Expr* expr, const SymbolEnvironment& env, int64_t& out, std::string& error){ Value v; if(!evalSymbol(expr,env,v,error)) return false; if(auto p=std::get_if<int64_t>(&v)){out=*p;return true;} if(auto p=std::get_if<uint64_t>(&v)){if(*p>uint64_t(std::numeric_limits<int64_t>::max())){error="integer value out of signed range";return false;}out=static_cast<int64_t>(*p);return true;} error="expression is not an integer";return false; }

bool evaluateSize(const core::Expr* expr, const SymbolEnvironment& env, LayoutSize& out, std::string& error){ Value v; if(!evalSymbol(expr,env,v,error)) return false; uint64_t x=0; if(auto unsignedValue=std::get_if<uint64_t>(&v))x=*unsignedValue; else if(auto signedValue=std::get_if<int64_t>(&v)){if(*signedValue<0){error="size/offset cannot be negative";return false;}x=static_cast<uint64_t>(*signedValue);} else {error="size expression is not an integer";return false;} out=x; return true; }

} // namespace embx::runtime
