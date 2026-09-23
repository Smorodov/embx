#include "decoder/Decoder.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <iterator>
#include <optional>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include "diagnostics/Diagnostic.h"
#include "runtime/Assertion.h"

namespace embx::decoder {
namespace {

bool toHostSize(runtime::LayoutSize n, std::size_t& out, std::string& err) {
    if (n > static_cast<runtime::LayoutSize>(std::numeric_limits<std::size_t>::max())) { err = "layout size exceeds host size_t"; return false; }
    out = static_cast<std::size_t>(n); return true;
}

using Env = runtime::SymbolEnvironment;

runtime::Value toRuntime(const Value& v) {
    if (auto p=std::get_if<int64_t>(&v.data)) return *p;
    if (auto p=std::get_if<uint64_t>(&v.data)) return *p;
    if (auto p=std::get_if<double>(&v.data)) return *p;
    if (auto p=std::get_if<bool>(&v.data)) return *p;
    return int64_t(0);
}

bool sameNumber(const runtime::Value& a,const runtime::Value& b){
    if(auto x=std::get_if<int64_t>(&a)){
        if(auto y=std::get_if<int64_t>(&b)) return *x==*y;
        if(auto y=std::get_if<uint64_t>(&b)) return *x>=0 && static_cast<uint64_t>(*x)==*y;
        if(auto y=std::get_if<double>(&b)) return static_cast<double>(*x)==*y;
    }
    if(auto x=std::get_if<uint64_t>(&a)){
        if(auto y=std::get_if<uint64_t>(&b)) return *x==*y;
        if(auto y=std::get_if<int64_t>(&b)) return *y>=0 && *x==static_cast<uint64_t>(*y);
        if(auto y=std::get_if<double>(&b)) return static_cast<double>(*x)==*y;
    }
    if(auto x=std::get_if<double>(&a)){
        if(auto y=std::get_if<double>(&b)) return *x==*y;
        if(auto y=std::get_if<int64_t>(&b)) return *x==static_cast<double>(*y);
        if(auto y=std::get_if<uint64_t>(&b)) return *x==static_cast<double>(*y);
    }
    if(auto x=std::get_if<bool>(&a)) if(auto y=std::get_if<bool>(&b)) return *x==*y;
    return false;
}

bool eval(const plan::Expr* e, const Env& env, runtime::Value& out, std::string& err) {
    if (!e) { err="missing expression"; return false; }
    return runtime::evaluate(e, env, out, err);
}

bool evalSize(const plan::Expr* e,const Env& env,runtime::LayoutSize& n,std::string& err){
    runtime::Value v; if(!eval(e,env,v,err))return false;
    if(auto p=std::get_if<int64_t>(&v)){
        if(*p<0){err="negative size";return false;}
        const auto x=static_cast<uint64_t>(*p);
        n=x;return true;}
    if(auto p=std::get_if<uint64_t>(&v)){n=*p;return true;}
    err="size expression is not an integer";return false;
}

bool evalSizeAtNext(const plan::Expr* e, const Env& env, runtime::LayoutSize next,
                    runtime::LayoutSize& n, std::string& err) {
    Env scoped = env;
    scoped[core::BuiltinNextSymbolId] = runtime::Value(next);
    return evalSize(e, scoped, n, err);
}

plan::Type cloneType(const plan::Type& src) { return src; }

class Decoder {
    const plan::Module& p; const runtime::CallbackRegistry* cbs; const Options& opt;
    runtime::Reader& r; Env env; size_t base=0; size_t recursionDepth=0; std::string path; diagnostics::Diagnostic diag;
    static runtime::Endian rtEndian(plan::Endian e){return e==plan::Endian::Big?runtime::Endian::Big:e==plan::Endian::Native?runtime::Endian::Native:runtime::Endian::Little;}
    runtime::Environment callbackEnvironment() const { runtime::Environment out; for (const auto& kv: env) { if (const auto* sym=p.symbolTable.find(kv.first)) out[sym->name]=kv.second; } return out; }
    const plan::Struct* structFor(const plan::Type& t) const { if(!t.reference.valid()) return nullptr; auto it=p.structIndexBySymbol.find(t.reference.id); return it==p.structIndexBySymbol.end()?nullptr:&p.structs[it->second]; }
    const plan::Enum* enumFor(const plan::Type& t) const { if(!t.reference.valid()) return nullptr; auto it=p.enumIndexBySymbol.find(t.reference.id); return it==p.enumIndexBySymbol.end()?nullptr:&p.enums[it->second]; }

    bool readType(const plan::Type& t, Value& out, std::optional<runtime::LayoutSize> boundedBytes, std::string& err) {
        plan::Type effective=cloneType(t);
        if (!effective.terminator.empty()) {
            if (effective.kind != core::TypeKind::Bytes || !effective.maxPayload.has_value()) { err="invalid terminated sequence type"; return false; }
            const std::size_t termSize = effective.terminator.size();
            if (*effective.maxPayload > static_cast<runtime::LayoutSize>(std::numeric_limits<std::size_t>::max() - termSize)) { err="terminated sequence maximum exceeds host size"; return false; }
            const std::size_t scanLimit = static_cast<std::size_t>(*effective.maxPayload) + termSize;
            const auto saved = r.state();
            std::vector<std::uint8_t> scanned;
            scanned.reserve(std::min(scanLimit, r.remaining()));
            for (std::size_t i = 0; i < scanLimit && r.remaining() != 0; ++i) {
                const auto byte = static_cast<std::uint8_t>(r.uint(1));
                scanned.push_back(byte);
                if (scanned.size() >= termSize && std::equal(effective.terminator.rbegin(), effective.terminator.rend(), scanned.rbegin())) {
                    scanned.resize(scanned.size() - termSize);
                    out = std::move(scanned);
                    return true;
                }
            }
            r.restore(saved);
            err="terminated sequence terminator not found within maximum payload length";
            return false;
        }
        if(effective.name=="bytes" || effective.name=="string") {
            std::optional<runtime::LayoutSize> n;
            if(!effective.dimensions.empty() && effective.dimensions[0].kind==core::Dimension::Kind::Remaining) {
                // [*] consumes what is actually left from the current reader
                // position to its active limit. The enclosing block size is not
                // necessarily equal to the remaining size because earlier members
                // may already have consumed bytes.
                n=r.remaining();
            }
            else if(!effective.dimensions.empty() && effective.dimensions[0].expression) { runtime::LayoutSize x=0; if(!evalSize(effective.dimensions[0].expression.get(),env,x,err)) return false; n=x; }
            else n=boundedBytes;
            if(!n.has_value()) { err="unsized byte/string field: "+effective.name; return false; }
            if(*n > static_cast<runtime::LayoutSize>(opt.maxAllocationBytes)){ err="byte/string allocation exceeds decoder limit"; return false; }
            std::size_t hostN=0; if(!toHostSize(*n,hostN,err)) return false;
            if (hostN > r.remaining()) { err="byte/string read beyond reader limit"; return false; }
            try {
                auto b=r.bytes(hostN);
                if(effective.name=="bytes") out=std::move(b);
                else out=std::string(b.begin(),b.end());
                return true;
            } catch(const std::exception& ex){ err=ex.what(); return false; }
        }

        const bool hasArrayDimensions = !effective.dimensions.empty();
        runtime::LayoutSize count=1;
        runtime::LayoutSize firstCount=0;
        for(std::size_t di=0; di<effective.dimensions.size(); ++di){
            const auto& d=effective.dimensions[di];
            if(d.kind!=core::Dimension::Kind::Fixed && d.kind!=core::Dimension::Kind::Dynamic) { err="remaining dimension is only valid for bytes/string"; return false; }
            if(!d.expression) { err="dynamic array dimension has no expression"; return false; }
            runtime::LayoutSize n=0; if(!evalSize(d.expression.get(),env,n,err)) return false;
            if(di==0) firstCount=n;
            if(n>opt.maxArrayElements){err="array exceeds decoder limit";return false;}
            if(n!=0 && count>std::numeric_limits<runtime::LayoutSize>::max()/n){err="array size overflow";return false;}
            count*=n;
            if(count > static_cast<runtime::LayoutSize>(opt.maxArrayElements)){err="array exceeds decoder limit";return false;}
        }
        if(hasArrayDimensions){
            if(count > static_cast<runtime::LayoutSize>(opt.maxAllocationBytes / (sizeof(Value) ? sizeof(Value) : 1))){err="array allocation exceeds decoder limit";return false;}
            std::size_t hostCount=0; if(!toHostSize(firstCount,hostCount,err)) return false;
            Value::Array a; a.reserve(hostCount);
            plan::Type element = cloneType(effective);
            element.dimensions.erase(element.dimensions.begin());
            for(std::size_t i=0;i<hostCount;++i){
                Value x;
                const std::string savedPath = path;
                path = savedPath + "[" + std::to_string(i) + "]";
                bool ok = readType(element,x,std::nullopt,err);
                if(!ok) return false;
                path=savedPath;
                a.push_back(std::move(x));
            }
            out=std::move(a);return true;
        }
        if(effective.kind==core::TypeKind::Named){
            if(auto sp=structFor(effective)) return readStruct(*sp,out,err);
            if(auto ep=enumFor(effective)){
                plan::Type u=cloneType(ep->underlying);
                if(u.name.empty()){u.kind=core::TypeKind::Primitive;u.name="u32";}
                return readScalar(u,out,err);
            }
            err="unknown named type: "+effective.name;return false;
        }
        return readScalar(effective,out,err);
    }
    bool readScalar(const plan::Type& t,Value& out,std::string& err){
        try {
            if(t.name=="u8"){out=uint64_t(r.uint(1));return true;} if(t.name=="u16"){out=uint64_t(r.uint(2));return true;} if(t.name=="u32"){out=uint64_t(r.uint(4));return true;} if(t.name=="u64"){out=uint64_t(r.uint(8));return true;}
            if(t.name=="i8"){out=int64_t(r.sint(1));return true;} if(t.name=="i16"){out=int64_t(r.sint(2));return true;} if(t.name=="i32"){out=int64_t(r.sint(4));return true;} if(t.name=="i64"){out=int64_t(r.sint(8));return true;}
            if(t.name=="f32"){out=double(r.f32());return true;} if(t.name=="f64"){out=double(r.f64());return true;}
            err="unsupported scalar type: "+t.name;return false;
        } catch(const std::exception& ex){err=ex.what();return false;}
    }
    bool field(const plan::Field& f,Value& out,std::string& err){
        r.endian(rtEndian(f.endian));
        std::optional<runtime::LayoutSize> bound;
        if(f.staticLength) bound=*f.staticLength;
        if(!readType(f.type,out,bound,err)) return false;
        if(f.transform){
            if(f.transform->name != "scale" || !std::isfinite(f.transform->factor) || f.transform->factor == 0.0){ err="invalid scale transform: "+f.name; return false; }
            double wire=0.0;
            if(auto p=std::get_if<int64_t>(&out.data)){
                if(*p > 9007199254740992LL || *p < -9007199254740992LL){err="scale on i64 requires an exactly representable logical input: "+f.name;return false;}
                wire=static_cast<double>(*p);
            } else if(auto p=std::get_if<uint64_t>(&out.data)){
                if(*p > 9007199254740992ULL){err="scale on u64 requires an exactly representable logical input: "+f.name;return false;}
                wire=static_cast<double>(*p);
            } else if(auto p=std::get_if<double>(&out.data)) wire=*p; else { err="scale requires numeric wire value: "+f.name; return false; }
            const double logical=wire*f.transform->factor;
            if(!std::isfinite(logical)){err="scale transform produced non-finite value: "+f.name;return false;}
            out=Value(logical);
        }
        if(!f.assertion.empty() && !runtime::assertionMatches(out,f.assertion,err)){
            if(err.empty()) err="assertion mismatch: "+f.name; else err="assertion mismatch: "+f.name+": "+err;
            return false;
        }
        return true;
    }
    bool members(const std::vector<std::unique_ptr<plan::Op>>& ms, Value::Object& obj,std::string& err){
        const std::string rootPath = path;
        for(const auto& op:ms){
            path = rootPath;
            if(!op){err="null plan operation"; if(!diag) diag=diagnostics::make(diagnostics::Code::Internal,err,path,r.pos()); return false;}
            if(!member(*op,obj,err)) return false;
        }
        path = rootPath;
        return true;
    }
    bool member(const plan::Op& op,Value::Object& obj,std::string& err){
        const std::string savedPath = path;
        auto setPath = [&](const std::string& component){ path = savedPath.empty() ? component : savedPath + "." + component; };
        try {
            switch(op.kind){
            case plan::OpKind::Virtual:{const auto& v=static_cast<const plan::Virtual&>(op);setPath(v.name);runtime::Value value;if(!eval(v.expression.get(),env,value,err))return false;env[v.symbol]=value;if(auto p=std::get_if<int64_t>(&value))obj[v.name]=*p;else if(auto p=std::get_if<uint64_t>(&value))obj[v.name]=*p;else if(auto p=std::get_if<double>(&value))obj[v.name]=*p;else if(auto p=std::get_if<bool>(&value))obj[v.name]=*p;return true;}
            case plan::OpKind::Alias:{const auto& a=static_cast<const plan::FieldAlias&>(op);setPath(a.name);auto it=env.find(a.target);if(it==env.end()){err="alias target is unavailable: "+a.targetName;return false;}if(auto p=std::get_if<int64_t>(&it->second))obj[a.name]=*p;else if(auto p=std::get_if<uint64_t>(&it->second))obj[a.name]=*p;else if(auto p=std::get_if<double>(&it->second))obj[a.name]=*p;else if(auto p=std::get_if<bool>(&it->second))obj[a.name]=*p;else {err="alias target has unsupported runtime value: "+a.targetName;return false;}env[a.symbol]=it->second;return true;}
            case plan::OpKind::Field:{auto& f=static_cast<const plan::Field&>(op); setPath(f.name);Value v;if(!field(f,v,err))return false;obj[f.name]=v;env[f.symbol]=toRuntime(v); return true;}
            case plan::OpKind::Bits:{auto& b=static_cast<const plan::Bits&>(op); setPath("<bits>");r.beginBits(static_cast<std::size_t>(b.totalBits));for(const auto& f:b.fields){r.endian(rtEndian(f.endian));auto v=uint64_t(r.bits(static_cast<std::size_t>(f.bits))); Value bv=v; if(!f.assertion.empty() && !runtime::assertionMatches(bv,f.assertion,err)){r.endBits();err="assertion mismatch: "+f.name;return false;} obj[f.name]=v;env[f.symbol]=v;}r.endBits();return true;}
            case plan::OpKind::Align:{auto& a=static_cast<const plan::Align&>(op); setPath("<align>");runtime::LayoutSize logical=a.staticAlignment.value_or(0);if(!a.staticAlignment&&!evalSize(a.alignment.get(),env,logical,err))return false;std::size_t n=0;if(!toHostSize(logical,n,err))return false;r.align(n);return true;}
            case plan::OpKind::At:{auto& a=static_cast<const plan::At&>(op); setPath("<at>");runtime::LayoutSize off=0;if(a.staticOffset)off=*a.staticOffset;else if(!evalSizeAtNext(a.offset.get(),env,static_cast<runtime::LayoutSize>(r.pos()),off,err))return false;if(off>std::numeric_limits<std::size_t>::max()-static_cast<std::size_t>(base)){err="at offset exceeds host address space";return false;}std::size_t hostOff=0;if(!toHostSize(off,hostOff,err))return false;std::size_t saved=r.pos();r.seekRoot(static_cast<std::size_t>(base)+hostOff);Value::Object nested;if(!members(a.members,nested,err)){r.seek(saved);return false;}r.seek(saved);for(auto& kv:nested)obj[kv.first]=std::move(kv.second);return true;}
            case plan::OpKind::Block:{auto& b=static_cast<const plan::Block&>(op); setPath(b.name.empty()?"<block>":b.name);runtime::LayoutSize logical=0;if(b.staticSize)logical=*b.staticSize;else if(!evalSize(b.size.get(),env,logical,err))return false;std::size_t n=0;if(!toHostSize(logical,n,err))return false;if(n>r.remaining()){err="block exceeds reader limit";return false;}std::size_t start=r.pos();std::size_t end=start+n;r.pushLimit(n);Value::Object nested;if(!members(b.members,nested,err)){r.popLimit();r.seek(start);return false;}if(r.pos()>end){r.popLimit();r.seek(start);err="block members exceed block size";return false;}r.seek(end);r.popLimit();for(auto& kv:nested)obj[kv.first]=std::move(kv.second);return true;}
            case plan::OpKind::Variant:{auto& v=static_cast<const plan::Variant&>(op); setPath(v.name);runtime::Value tag;if(!eval(v.discriminator.get(),env,tag,err))return false;Value::Object nested;const plan::VariantCase* chosen=nullptr;for(const auto& c:v.cases){if(sameNumber(c.tag,tag)){chosen=&c;break;}}if(chosen){if(chosen->type){Value x;if(!readType(*chosen->type,x,std::nullopt,err))return false;nested["value"]=std::move(x);}else if(!members(chosen->members,nested,err))return false;}else if(v.hasDefault){if(v.defaultType){Value x;if(!readType(*v.defaultType,x,std::nullopt,err))return false;nested["value"]=std::move(x);}else if(!members(v.defaultMembers,nested,err))return false;}else{err="no variant case for discriminator";return false;}obj[v.name]=std::move(nested);return true;}
            case plan::OpKind::Conditional:{
                const auto& c=static_cast<const plan::Conditional&>(op); setPath("<if>");
                runtime::Value condition;
                if(!eval(c.condition.get(),env,condition,err)) return false;
                const auto* selected=std::get_if<bool>(&condition);
                if(!selected){err="conditional condition did not evaluate to boolean";return false;}
                Value::Object nested;
                if(!members(*selected?c.thenMembers:c.elseMembers,nested,err)) return false;
                for(auto& kv:nested) obj[kv.first]=std::move(kv.second);
                return true;
            }
            case plan::OpKind::Callback:{auto& cb=static_cast<const plan::Callback&>(op); setPath(cb.name);if(cb.direction != core::CallbackDirection::Decode){err="callback direction is not valid for decoder: "+cb.name;return false;}std::vector<runtime::Value> args;for(const auto& e:cb.args){runtime::Value x;if(!eval(e.get(),env,x,err))return false;args.push_back(x);}if(!cbs){err="callback registry is not configured: "+cb.name;return false;}auto cr=cbs->call(cb.name,r,callbackEnvironment(),args);if(!cr.success){err=cr.error;return false;}return true;}
            }
        } catch(const std::exception& ex){err=ex.what(); if(!diag) diag=diagnostics::make(diagnostics::classify(err,false),err,path,r.pos()); return false;} return false;
    }
    void addSizeProperties(const plan::Struct& s) {
        env[core::BuiltinMinSizeInBytesSymbolId] = runtime::Value(s.minSizeInBytes);
        if (s.sizeInBytes) env[core::BuiltinSizeInBytesSymbolId] = runtime::Value(*s.sizeInBytes);
        if (s.maxSizeInBytes) env[core::BuiltinMaxSizeInBytesSymbolId] = runtime::Value(*s.maxSizeInBytes);
    }
    bool readStruct(const plan::Struct& s,Value& out,std::string& err){
        if(++recursionDepth > opt.maxRecursionDepth){ --recursionDepth; err="maximum recursion depth exceeded"; return false; }
        size_t start=r.pos();
        size_t oldBase=base;
        Env saved=env;
                addSizeProperties(s);
                r.endian(rtEndian(s.endian));
        Value::Object o;
        bool ok = members(s.members,o,err);
        if (ok) {
            for (const auto& requirement : s.requirements) {
                runtime::Value condition;
                if (!eval(requirement.get(), env, condition, err)) { ok=false; break; }
                const auto* passed = std::get_if<bool>(&condition);
                if (!passed || !*passed) { err="requires constraint failed"; ok=false; break; }
            }
        }
        if(!ok){
            // Preserve the failure location before transactional rollback. Reader::seek(start)
            // is required for composability, while the diagnostic points at the final check.
            const size_t failureOffset=r.pos();
            if(!diag) diag=diagnostics::make(diagnostics::classify(err,false),err,path,failureOffset);
            r.seek(start);
            base=oldBase;
            env=saved;
            --recursionDepth;
            return false;
        }
        base=oldBase;
        env=saved;
                out=std::move(o);
        --recursionDepth;
        return true;
    }
public:
    const diagnostics::Diagnostic& diagnostic() const noexcept { return diag; }
    Decoder(const plan::Module& p_,const runtime::CallbackRegistry* c,const Options& o,runtime::Reader& rr):p(p_),cbs(c),opt(o),r(rr){}
    bool run(const plan::Struct& s,Value& out,std::string& err){
        env = opt.parameters;
        for (const auto& pmeta : p.parameters) {
            const auto it = env.find(pmeta.symbol);
            if (it == env.end()) { err="missing parameter: "+pmeta.name; return false; }
            const auto& n = pmeta.type.name;
            const bool okType = (n.size() > 1 && n[0] == 'u' && std::holds_alternative<uint64_t>(it->second)) ||
                                (n.size() > 1 && n[0] == 'i' && std::holds_alternative<int64_t>(it->second)) ||
                                ((n == "f32" || n == "f64") && std::holds_alternative<double>(it->second));
            if (!okType) { err="parameter type mismatch: "+pmeta.name; return false; }
            if (n[0] == 'u') { const unsigned bits=static_cast<unsigned>(std::stoul(n.substr(1))); const auto v=std::get<uint64_t>(it->second); if (bits < 64 && v >= (uint64_t(1) << bits)) { err="parameter value out of range: "+pmeta.name; return false; } }
            if (n[0] == 'i') { const unsigned bits=static_cast<unsigned>(std::stoul(n.substr(1))); const auto v=std::get<int64_t>(it->second); if (bits < 64) { const int64_t lo=-(int64_t(1) << (bits-1)), hi=(int64_t(1) << (bits-1))-1; if (v < lo || v > hi) { err="parameter value out of range: "+pmeta.name; return false; } } }
        }
        for (const auto& kv : env) if (!p.symbolTable.find(kv.first) || p.symbolTable.find(kv.first)->kind != core::SymbolKind::Parameter) { err="extra parameter SymbolId"; return false; }
        // Module constants and enum items are compile-time values materialized
        // in Plan. They are part of every executable struct environment; only
        // caller-supplied values are restricted to parameters above.
        for (const auto& c : p.constants) env[c.symbol] = c.value;
        for (const auto& e : p.enums) for (const auto& item : e.items) env[item.symbol] = item.value;
        path=s.name;
        bool ok=readStruct(s,out,err);
        if(!ok && !diag) diag=diagnostics::make(diagnostics::classify(err,false),err,path,r.pos());
        return ok;
    }
};
}

Engine::Engine(const plan::Module& plan,Options options):plan_(plan),options_(options){}
Result Engine::decode(const std::string& structName,const std::vector<uint8_t>& input) const {
    Result z;
    auto sid=plan_.symbolTable.findId(structName);
    auto it=plan_.structIndexBySymbol.find(sid);
    if(sid==core::InvalidSymbolId || it==plan_.structIndexBySymbol.end()){ z.error="unknown struct: "+structName; z.diagnostic=diagnostics::make(diagnostics::Code::UnknownStruct,z.error,structName,0); return z; }
    runtime::Reader r(input); Decoder d(plan_,callbacks_,options_,r); std::string err;
    if(!d.run(plan_.structs[it->second],z.value,err)){ z.error=err; z.consumed=r.pos(); z.diagnostic=d.diagnostic() ? d.diagnostic() : diagnostics::make(diagnostics::classify(err,false),err,structName,z.consumed); if(z.diagnostic.path.empty()) z.diagnostic.path=structName; return z; }
    z.consumed=r.pos();
    if(options_.requireFullInput&&r.pos()!=input.size()){ z.error="trailing input: "+std::to_string(input.size()-r.pos())+" bytes"; z.diagnostic=diagnostics::make(diagnostics::Code::InputTruncated,z.error,structName,z.consumed,input.size(),r.pos(),true); return z; }
    z.success=true; return z;
}
} // namespace embx::decoder
