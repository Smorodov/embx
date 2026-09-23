#include "encoder/Encoder.h"
#include "runtime/ExpressionEvaluator.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include "diagnostics/Diagnostic.h"
#include "runtime/Assertion.h"

namespace embx::encoder {

namespace {

bool toHostSize(embx::runtime::LayoutSize n, std::size_t& out, std::string& err) {
    if (n > static_cast<embx::runtime::LayoutSize>(std::numeric_limits<std::size_t>::max())) { err = "layout size exceeds host size_t"; return false; }
    out = static_cast<std::size_t>(n); return true;
}
using Env=runtime::SymbolEnvironment;

plan::Type cloneType(const plan::Type& s){ return s; }

bool eval(const plan::Expr* e,const Env& env,runtime::Value& out,std::string& err){
    return runtime::evaluate(e, env, out, err);
}
bool evalSize(const plan::Expr* e,const Env& env,embx::runtime::LayoutSize& n,std::string& err){
    runtime::Value v;if(!eval(e,env,v,err))return false;
    if(auto p=std::get_if<int64_t>(&v)){if(*p<0){err="negative size";return false;}n=static_cast<embx::runtime::LayoutSize>(*p);return true;}
    if(auto p=std::get_if<uint64_t>(&v)){n=*p;return true;}
    err="size expression is not an integer";return false;
}


bool evalSizeAtNext(const plan::Expr* e, const Env& env, embx::runtime::LayoutSize next,
                    embx::runtime::LayoutSize& n, std::string& err) {
    Env scoped = env;
    scoped[core::BuiltinNextSymbolId] = runtime::Value(next);
    return evalSize(e, scoped, n, err);
}

runtime::Value runtimeValue(const Value& v){
    if(auto p=std::get_if<int64_t>(&v.data))return *p;
    if(auto p=std::get_if<uint64_t>(&v.data))return *p;
    if(auto p=std::get_if<double>(&v.data))return *p;
    if(auto p=std::get_if<bool>(&v.data))return *p;
    return int64_t(0);
}

Value valueFromRuntime(const runtime::Value& v){
    if(auto p=std::get_if<int64_t>(&v))return Value(*p);
    if(auto p=std::get_if<uint64_t>(&v))return Value(*p);
    if(auto p=std::get_if<double>(&v))return Value(*p);
    if(auto p=std::get_if<bool>(&v))return Value(*p);
    return Value();
}
bool asU64(const Value& v,uint64_t& x){
    if(auto p=std::get_if<uint64_t>(&v.data)){x=*p;return true;}
    if(auto p=std::get_if<int64_t>(&v.data)){if(*p<0)return false;x=static_cast<uint64_t>(*p);return true;}
    return false;
}
bool asI64(const Value& v,int64_t& x){
    if(auto p=std::get_if<int64_t>(&v.data)){x=*p;return true;}
    if(auto p=std::get_if<uint64_t>(&v.data)){if(*p>uint64_t(std::numeric_limits<int64_t>::max()))return false;x=static_cast<int64_t>(*p);return true;}
    return false;
}
bool asNumber(const Value& v,double& x){
    if(auto p=std::get_if<double>(&v.data)){x=*p;return std::isfinite(*p);}
    if(auto p=std::get_if<int64_t>(&v.data)){x=double(*p);return true;}
    if(auto p=std::get_if<uint64_t>(&v.data)){x=double(*p);return true;}
    return false;
}
bool sameNumber(const runtime::Value&a,const runtime::Value&b){
    if(auto x=std::get_if<int64_t>(&a)){if(auto y=std::get_if<int64_t>(&b))return *x==*y;if(auto y=std::get_if<uint64_t>(&b))return *x>=0&&uint64_t(*x)==*y;if(auto y=std::get_if<double>(&b))return double(*x)==*y;}
    if(auto x=std::get_if<uint64_t>(&a)){if(auto y=std::get_if<uint64_t>(&b))return *x==*y;if(auto y=std::get_if<int64_t>(&b))return *y>=0&&*x==uint64_t(*y);if(auto y=std::get_if<double>(&b))return double(*x)==*y;}
    if(auto x=std::get_if<double>(&a)){if(auto y=std::get_if<double>(&b))return *x==*y;if(auto y=std::get_if<int64_t>(&b))return *x==double(*y);if(auto y=std::get_if<uint64_t>(&b))return *x==double(*y);}
    if(auto x=std::get_if<bool>(&a))if(auto y=std::get_if<bool>(&b))return *x==*y;
    return false;
}

class Writer {
    std::vector<uint8_t> d_; size_t p_=0; runtime::Endian e_=runtime::Endian::Little; size_t limit_=0; std::vector<size_t> limits_;
    uint64_t bits_=0; size_t bitCount_=0; size_t bitTotal_=0;
    static bool hostLittle(){uint16_t x=1;return *reinterpret_cast<uint8_t*>(&x)==1;}
    bool little()const{return e_==runtime::Endian::Little||(e_==runtime::Endian::Native&&hostLittle());}
    void ensure(size_t n){if(n>limit_-p_)throw std::out_of_range("write beyond active limit");}
public:
    explicit Writer(size_t reserve=0){d_.reserve(reserve);limit_=std::numeric_limits<size_t>::max();}
    size_t pos()const{return p_;} const std::vector<uint8_t>& data()const{return d_;} std::vector<uint8_t>& dataMutable(){return d_;} void setPos(size_t p){if(p>limit_)throw std::out_of_range("callback position beyond writer limit"); if(p>d_.size())d_.resize(p,0);p_=p;}
    void endian(runtime::Endian e){e_=e;} runtime::Endian endian()const{return e_;}
    void seek(size_t p){if(p>limit_)throw std::out_of_range("seek beyond writer limit"); if(p>d_.size())d_.resize(p,0);p_=p;}
    void seekRoot(size_t p){if(p>limit_)throw std::out_of_range("root seek beyond writer limit");if(p>d_.size())d_.resize(p,0);p_=p;}
    void align(size_t a){if(!a)throw std::invalid_argument("alignment is zero");size_t rem=p_%a;size_t n=rem? a-rem:0;writeZeros(n);}
    void writeZeros(size_t n){ensure(n);if(n>std::numeric_limits<size_t>::max()-p_)throw std::overflow_error("write position overflow");if(d_.size()<p_+n)d_.resize(p_+n,0);else std::fill(d_.begin()+static_cast<std::vector<uint8_t>::difference_type>(p_), d_.begin()+static_cast<std::vector<uint8_t>::difference_type>(p_+n), 0);p_+=n;}
    void bytes(const std::vector<uint8_t>& b){ensure(b.size());if(d_.size()<p_+b.size())d_.resize(p_+b.size());std::copy(b.begin(),b.end(),d_.begin()+static_cast<std::vector<uint8_t>::difference_type>(p_));p_+=b.size();}
    void bytes(const std::string& s){ensure(s.size());if(d_.size()<p_+s.size())d_.resize(p_+s.size());std::copy(s.begin(),s.end(),d_.begin()+static_cast<std::vector<uint8_t>::difference_type>(p_));p_+=s.size();}
    void uint(uint64_t v,size_t n){if(n==0||n>8)throw std::invalid_argument("integer width must be 1..8 bytes");ensure(n);if(d_.size()<p_+n)d_.resize(p_+n);if(little()){for(size_t i=0;i<n;++i)d_[p_+i]=uint8_t(v>>(8*i));}else{for(size_t i=0;i<n;++i)d_[p_+n-1-i]=uint8_t(v>>(8*i));}p_+=n;}
    void sint(int64_t v,size_t n){uint(uint64_t(v),n);}
    void f32(float v){uint32_t u;std::memcpy(&u,&v,4);uint(u,4);} void f64(double v){uint64_t u;std::memcpy(&u,&v,8);uint(u,8);}
    void beginBits(size_t total){if(bitCount_)throw std::logic_error("bit container already active");if(!total||total>64)throw std::invalid_argument("bit container width must be 1..64");bitTotal_=total;bitCount_=0;bits_=0;}
    void bit(uint64_t v,size_t width){if(!bitTotal_)throw std::logic_error("no active bit container");if(!width||width>bitTotal_-bitCount_)throw std::out_of_range("bit write exceeds active bit container");if(width<64&&v>=(uint64_t(1)<<width))throw std::out_of_range("bit value exceeds field width");size_t shift=bitTotal_-bitCount_-width;bits_|=v<<shift;bitCount_+=width;}
    void endBits(){if(!bitTotal_||bitCount_!=bitTotal_)throw std::logic_error("bit container not fully written");size_t nb=(bitTotal_+7)/8;size_t cb=nb*8;uint64_t x=bits_;if(cb>bitTotal_)x<<=(cb-bitTotal_);ensure(nb);if(d_.size()<p_+nb)d_.resize(p_+nb);if(little()){for(size_t i=0;i<nb;++i)d_[p_+i]=uint8_t(x>>(8*i));}else{for(size_t i=0;i<nb;++i)d_[p_+nb-1-i]=uint8_t(x>>(8*i));}p_+=nb;bitTotal_=bitCount_=0;bits_=0;}
    void pushLimit(size_t n){if(n>limit_-p_)throw std::out_of_range("block exceeds writer limit");limits_.push_back(limit_);limit_=p_+n;}
    void popLimit(){if(limits_.empty())throw std::logic_error("no active writer limit");limit_=limits_.back();limits_.pop_back();}
    void truncate(size_t n){if(n>d_.size())d_.resize(n,0);else d_.resize(n);if(p_>n)p_=n;}
};

class Encoder {
    const plan::Module& p; const Options& opt; Writer w; Env env; size_t base=0; size_t recursionDepth=0; std::string path; diagnostics::Diagnostic diag;
    static runtime::Endian rt(plan::Endian e){return e==plan::Endian::Big?runtime::Endian::Big:e==plan::Endian::Native?runtime::Endian::Native:runtime::Endian::Little;}
    runtime::Environment callbackEnvironment() const { runtime::Environment out; for (const auto& kv: env) { if (const auto* sym=p.symbolTable.find(kv.first)) out[sym->name]=kv.second; } return out; }
    const plan::Struct* structFor(const plan::Type& t) const {
        if (!t.reference.valid()) return nullptr;
        const auto it = p.structIndexBySymbol.find(t.reference.id);
        return it == p.structIndexBySymbol.end() ? nullptr : &p.structs[it->second];
    }
    const plan::Enum* enumFor(const plan::Type& t) const {
        if (!t.reference.valid()) return nullptr;
        const auto it = p.enumIndexBySymbol.find(t.reference.id);
        return it == p.enumIndexBySymbol.end() ? nullptr : &p.enums[it->second];
    }
    const Value* objectField(const Value::Object& o,const std::string& n,std::string& err){auto it=o.find(n);if(it==o.end()){err="missing field: "+n;return nullptr;}return &it->second;}
    bool bindAliases(const std::vector<std::unique_ptr<plan::Op>>& ms,const Value::Object& o,std::string& err){
        for(const auto& op:ms){
            if(!op || op->kind!=plan::OpKind::Alias) continue;
            const auto& a=static_cast<const plan::FieldAlias&>(*op);
            const auto input=o.find(a.name);
            if(input==o.end()) continue;
            const auto* targetSymbol=p.symbolTable.find(a.target);
            if(!targetSymbol || targetSymbol->kind!=core::SymbolKind::Field){
                err="alias target is not writable: "+a.targetName;
                return false;
            }
            const bool supported = std::holds_alternative<int64_t>(input->second.data) ||
                                   std::holds_alternative<uint64_t>(input->second.data) ||
                                   std::holds_alternative<double>(input->second.data) ||
                                   std::holds_alternative<bool>(input->second.data);
            if(!supported){
                err="alias value has unsupported runtime type: "+a.name;
                return false;
            }
            const runtime::Value aliasValue=runtimeValue(input->second);
            const auto targetInput=o.find(a.targetName);
            if(targetInput!=o.end() && !sameNumber(runtimeValue(targetInput->second),aliasValue)){
                err="alias and target values disagree: "+a.name;
                return false;
            }
            const auto existing=env.find(a.target);
            if(existing!=env.end() && !sameNumber(existing->second,aliasValue)){
                err="alias and target values disagree: "+a.name;
                return false;
            }
            env[a.target]=aliasValue;
        }
        return true;
    }
    bool writeScalar(const plan::Type&t,const Value&v,std::string&err){try{
        if(t.name=="u8"||t.name=="u16"||t.name=="u32"||t.name=="u64"){uint64_t x;if(!asU64(v,x)){err="expected unsigned integer for "+t.name;return false;}size_t n=std::stoul(t.name.substr(1))/8;if(n<8&&x>=(uint64_t(1)<<(8*n))){err="integer value out of range for "+t.name;return false;}w.uint(x,n);return true;}
        if(t.name=="i8"||t.name=="i16"||t.name=="i32"||t.name=="i64"){int64_t x;if(!asI64(v,x)){err="expected signed integer for "+t.name;return false;}size_t n=std::stoul(t.name.substr(1))/8;if(n<8){int64_t lo=-(int64_t(1)<<(8*n-1)),hi=(int64_t(1)<<(8*n-1))-1;if(x<lo||x>hi){err="integer value out of range for "+t.name;return false;}}w.sint(x,n);return true;}
        if(t.name=="f32"){double x;if(!asNumber(v,x)){err="expected finite number for f32";return false;}float f=static_cast<float>(x);if(!std::isfinite(f)){err="f32 value is not finite";return false;}w.f32(f);return true;}
        if(t.name=="f64"){double x;if(!asNumber(v,x)){err="expected finite number for f64";return false;}w.f64(x);return true;}
        err="unsupported scalar type: "+t.name;return false;
    }catch(const std::exception&ex){err=ex.what();return false;}}
    bool writeType(const plan::Type&t,const Value&v,std::optional<embx::runtime::LayoutSize> bound,std::string&err){plan::Type e=cloneType(t);
        if (!e.terminator.empty()) {
            if (!e.maxPayload.has_value()) { err="invalid terminated sequence type"; return false; }
            const bool byteForm = e.kind == core::TypeKind::Bytes && e.dimensions.empty();
            const bool sequenceForm = e.dimensions.size() == 1 && e.dimensions[0].kind == core::Dimension::Kind::Remaining;
            if (!byteForm && !sequenceForm) { err="invalid terminated sequence type"; return false; }
            if (byteForm) {
                const auto* b = std::get_if<Value::Bytes>(&v.data);
                if (!b) { err="expected bytes for terminated sequence"; return false; }
                if (static_cast<embx::runtime::LayoutSize>(b->size()) > *e.maxPayload) { err="terminated sequence payload exceeds maximum length"; return false; }
                if (std::search(b->begin(), b->end(), e.terminator.begin(), e.terminator.end()) != b->end()) { err="terminated sequence payload contains its terminator"; return false; }
                if (bound && static_cast<embx::runtime::LayoutSize>(b->size()) + static_cast<embx::runtime::LayoutSize>(e.terminator.size()) > *bound) { err="terminated sequence exceeds field bound"; return false; }
                w.bytes(*b); w.bytes(e.terminator); return true;
            }
            const auto* a = std::get_if<Value::Array>(&v.data);
            if (!a) { err="expected array for terminated sequence"; return false; }
            const std::size_t start = w.pos();
            plan::Type element = cloneType(e);
            element.dimensions.clear();
            element.terminator = {};
            element.maxPayload.reset();
            for (std::size_t i = 0; i < a->size(); ++i) {
                const std::string savedPath = path;
                path = savedPath + "[" + std::to_string(i) + "]";
                if (!writeType(element, (*a)[i], std::nullopt, err)) { path = savedPath; return false; }
                path = savedPath;
                if (static_cast<embx::runtime::LayoutSize>(w.pos() - start) > *e.maxPayload) { err="terminated sequence payload exceeds maximum length"; return false; }
            }
            if (bound && static_cast<embx::runtime::LayoutSize>(w.pos() - start) + static_cast<embx::runtime::LayoutSize>(e.terminator.size()) > *bound) { err="terminated sequence exceeds field bound"; return false; }
            w.bytes(e.terminator); return true;
        }
        if(e.name=="bytes"||e.name=="string"){
            embx::runtime::LayoutSize logical=0;std::size_t n=0;bool fixed=false;if(!e.dimensions.empty()){if(e.dimensions[0].kind==core::Dimension::Kind::Fixed && e.dimensions[0].expression){if(!evalSize(e.dimensions[0].expression.get(),env,logical,err))return false;if(!toHostSize(logical,n,err))return false;fixed=true;}}
            if(auto b=std::get_if<Value::Bytes>(&v.data)){if(e.name!="bytes"){err="expected bytes for "+e.name;return false;}if(fixed&&static_cast<embx::runtime::LayoutSize>(b->size())!=logical){err="byte field length mismatch";return false;}if(bound&&static_cast<embx::runtime::LayoutSize>(b->size())>*bound){err="byte field exceeds bound";return false;}w.bytes(*b);return true;}
            if(auto s=std::get_if<std::string>(&v.data)){if(e.name!="string"){err="expected string for "+e.name;return false;}if(fixed&&static_cast<embx::runtime::LayoutSize>(s->size())!=logical){err="string field length mismatch";return false;}if(bound&&static_cast<embx::runtime::LayoutSize>(s->size())>*bound){err="string field exceeds bound";return false;}w.bytes(*s);return true;}
            err="expected bytes or string value";return false;
        }
        embx::runtime::LayoutSize count=1;embx::runtime::LayoutSize firstCount=0;for(std::size_t di=0;di<e.dimensions.size();++di){const auto& d=e.dimensions[di];if(d.kind!=core::Dimension::Kind::Fixed && d.kind!=core::Dimension::Kind::Dynamic){err="remaining dimension is only valid for bytes/string";return false;}if(!d.expression){err="dynamic array dimension has no expression";return false;}embx::runtime::LayoutSize n=0;if(!evalSize(d.expression.get(),env,n,err))return false;if(di==0) firstCount=n;if(n!=0&&count>std::numeric_limits<embx::runtime::LayoutSize>::max()/n){err="array size overflow";return false;}count*=n;if(n>static_cast<embx::runtime::LayoutSize>(opt.maxArrayElements)||count>static_cast<embx::runtime::LayoutSize>(opt.maxArrayElements)){err="array exceeds encoder limit";return false;}}
        const bool hasArrayDimensions = !e.dimensions.empty();
        if(hasArrayDimensions){
            if(count > opt.maxAllocationBytes / (sizeof(Value) ? sizeof(Value) : 1)){err="array allocation exceeds encoder limit";return false;}
            auto a=std::get_if<Value::Array>(&v.data);
            if(!a){err="expected array for "+e.name;return false;}
            if(static_cast<embx::runtime::LayoutSize>(a->size())!=firstCount){err="array length mismatch";return false;}
            plan::Type element=cloneType(e);
            element.dimensions.erase(element.dimensions.begin());
            for(std::size_t i=0;i<a->size();++i){
                const auto& x=(*a)[i];
                const std::string savedPath=path;
                path=savedPath+"["+std::to_string(i)+"]";
                bool ok=writeType(element,x,std::nullopt,err);
                if(!ok) return false;
                path=savedPath;
            }
            return true;
        }
        if(e.kind==core::TypeKind::Named){
            if(auto sp=structFor(e)) return writeStruct(*sp,v,err);
            if(auto ep=enumFor(e)){
                plan::Type u=cloneType(ep->underlying);
                if(u.name.empty()){u.kind=core::TypeKind::Primitive;u.name="u32";}
                return writeScalar(u,v,err);
            }
            err="unknown named type: "+e.name;return false;
        }
        return writeScalar(e,v,err);
    }
    bool field(const plan::Field&f,const Value&v,std::string&err){
        w.endian(rt(f.endian));
        Value wire=v;
        if(!f.assertion.empty()&&!runtime::assertionMatches(v,f.assertion,err)){if(err.empty())err="assertion mismatch: "+f.name;else err="assertion mismatch: "+f.name+": "+err;return false;}
        if(f.transform){
            if(f.transform->name != "scale" || !std::isfinite(f.transform->factor) || f.transform->factor == 0.0){err="invalid scale transform: "+f.name;return false;}
            double logical=0.0;
            if(auto q=std::get_if<int64_t>(&v.data)) logical=static_cast<double>(*q); else if(auto q=std::get_if<uint64_t>(&v.data)) logical=static_cast<double>(*q); else if(auto q=std::get_if<double>(&v.data)) logical=*q; else {err="scale requires numeric logical value: "+f.name;return false;}
            const double x=logical/f.transform->factor;
            if(!std::isfinite(x)){err="scale transform produced non-finite wire value: "+f.name;return false;}
            const std::string& n=f.type.name;
            if(n.size()>1 && n[0]=='u'){
                if(x<0.0 || std::trunc(x)!=x){err="scale result is not exactly representable by "+n+": "+f.name;return false;}
                const unsigned bits=static_cast<unsigned>(std::stoul(n.substr(1)));
                const long double limit=std::ldexp(1.0L,bits);
                if(static_cast<long double>(x)>=limit){err="scale result is out of range for "+n+": "+f.name;return false;}
                wire=Value(static_cast<uint64_t>(x));
            } else if(n.size()>1 && n[0]=='i'){
                const unsigned bits=static_cast<unsigned>(std::stoul(n.substr(1)));
                const long double lo=-std::ldexp(1.0L,bits-1);
                const long double hi=std::ldexp(1.0L,bits-1);
                if(std::trunc(x)!=x || static_cast<long double>(x)<lo || static_cast<long double>(x)>=hi){err="scale result is not exactly representable by "+n+": "+f.name;return false;}
                wire=Value(static_cast<int64_t>(x));
            } else if(n=="f32"){
                const float f32=static_cast<float>(x);
                if(!std::isfinite(f32) || static_cast<double>(f32)!=x){err="scale result is not exactly representable by f32: "+f.name;return false;}
                wire=Value(static_cast<double>(f32));
            } else wire=Value(x);
        }
        std::optional<embx::runtime::LayoutSize> bound=f.staticLength;std::size_t before=w.pos();if(!writeType(f.type,wire,bound,err))return false;if(f.staticLength&&static_cast<embx::runtime::LayoutSize>(w.pos()-before)!=*f.staticLength){err="field encoded length mismatch: "+f.name;return false;}return true;
    }
    bool members(const std::vector<std::unique_ptr<plan::Op>>&ms,const Value::Object&o,std::string&err){
        const std::string rootPath=path;
        if(!bindAliases(ms,o,err)) return false;
        for(const auto&op:ms){
            path=rootPath;
            if(!op){err="null plan operation";if(!diag)diag=diagnostics::make(diagnostics::Code::Internal,err,path,w.pos());return false;}
            if(!member(*op,o,err))return false;
        }
        path=rootPath; return true;
    }
    bool member(const plan::Op&op,const Value::Object&o,std::string&err){
        const std::string savedPath=path;
        auto setPath=[&](const std::string& q){path=savedPath.empty()?q:savedPath+"."+q;};
        switch(op.kind){
        case plan::OpKind::Virtual:{
            const auto& v=static_cast<const plan::Virtual&>(op); setPath(v.name); runtime::Value value; if(!eval(v.expression.get(),env,value,err)) return false; env[v.symbol]=value; return true;
        }
        case plan::OpKind::Alias:{const auto& a=static_cast<const plan::FieldAlias&>(op);setPath(a.name);auto it=env.find(a.target);if(it==env.end()){err="alias target is unavailable: "+a.targetName;return false;}env[a.symbol]=it->second;return true;}
        case plan::OpKind::Field:{auto&f=static_cast<const plan::Field&>(op); setPath(f.name);const Value* v=objectField(o,f.name,err);if(!v){auto it=env.find(f.symbol);if(it==env.end())return false;Value aliased=valueFromRuntime(it->second);if(!field(f,aliased,err))return false;env[f.symbol]=it->second;return true;}if(!field(f,*v,err))return false;env[f.symbol]=runtimeValue(*v); return true;}
        case plan::OpKind::Bits:{auto&b=static_cast<const plan::Bits&>(op); setPath("<bits>");if(b.fields.empty()){err="empty bits block";return false;} w.endian(rt(b.fields.front().endian)); w.beginBits(static_cast<std::size_t>(b.totalBits));for(const auto&f:b.fields){auto v=objectField(o,f.name,err);if(!v){w.endBits();return false;}uint64_t x;if(!asU64(*v,x)){err="expected unsigned integer for bit field: "+f.name;w.endBits();return false;} Value bv=x; if(!f.assertion.empty()&&!runtime::assertionMatches(bv,f.assertion,err)){w.endBits();err="assertion mismatch: "+f.name;return false;} w.bit(x,static_cast<std::size_t>(f.bits));env[f.symbol]=x ;}w.endBits();return true;}
        case plan::OpKind::Align:{auto&a=static_cast<const plan::Align&>(op); setPath("<align>");embx::runtime::LayoutSize logical=0;if(a.staticAlignment)logical=*a.staticAlignment;else if(!evalSize(a.alignment.get(),env,logical,err))return false;std::size_t n=0;if(!toHostSize(logical,n,err))return false;w.align(n);return true;}
        case plan::OpKind::At:{auto&a=static_cast<const plan::At&>(op); setPath("<at>");embx::runtime::LayoutSize off=0;if(a.staticOffset)off=*a.staticOffset;else if(!evalSizeAtNext(a.offset.get(),env,static_cast<embx::runtime::LayoutSize>(w.pos()),off,err))return false;std::size_t hostOff=0;if(!toHostSize(off,hostOff,err))return false;if(hostOff>std::numeric_limits<std::size_t>::max()-base){err="at offset overflow";return false;}std::size_t saved=w.pos();std::size_t oldSize=w.data().size();std::size_t target=base+hostOff;std::vector<uint8_t> overwritten; if(target<oldSize) overwritten.assign(w.data().begin()+static_cast<std::vector<uint8_t>::difference_type>(target),w.data().end()); w.seekRoot(target);if(!members(a.members,o,err)){w.truncate(oldSize);if(target<oldSize) std::copy(overwritten.begin(),overwritten.end(),w.dataMutable().begin()+static_cast<std::vector<uint8_t>::difference_type>(target));w.seek(saved);return false;}w.seek(saved);return true;}
        case plan::OpKind::Block:{auto&b=static_cast<const plan::Block&>(op); setPath(b.name.empty()?"<block>":b.name);embx::runtime::LayoutSize logical=0;if(b.staticSize)logical=*b.staticSize;else if(!evalSize(b.size.get(),env,logical,err))return false;std::size_t n=0;if(!toHostSize(logical,n,err))return false;std::size_t start=w.pos();w.pushLimit(n);if(!members(b.members,o,err)){w.popLimit();return false;}size_t used=w.pos()-start;if(used>n){w.popLimit();err="block members exceed block size";return false;}if(used<n&&opt.allowTrailingBlockBytes)w.writeZeros(n-used);else if(used<n){w.popLimit();err="block members do not fill block size";return false;}w.popLimit();return true;}
        case plan::OpKind::Variant:{auto&v=static_cast<const plan::Variant&>(op); setPath(v.name);auto it=o.find(v.name);if(it==o.end()){err="missing variant: "+v.name;return false;}auto vo=std::get_if<Value::Object>(&it->second.data);if(!vo){err="variant must be an object: "+v.name;return false;}runtime::Value tag;if(!eval(v.discriminator.get(),env,tag,err))return false;const plan::VariantCase*chosen=nullptr;for(const auto&c:v.cases){if(sameNumber(c.tag,tag)){chosen=&c;break;}}if(chosen){if(chosen->type){auto q=vo->find("value");if(q==vo->end()){err="missing variant value: "+v.name;return false;}return writeType(*chosen->type,q->second,std::nullopt,err);}return members(chosen->members,*vo,err);}if(v.hasDefault){if(v.defaultType){auto q=vo->find("value");if(q==vo->end()){err="missing default variant value: "+v.name;return false;}return writeType(*v.defaultType,q->second,std::nullopt,err);}return members(v.defaultMembers,*vo,err);}err="no variant case for discriminator";return false;}
        case plan::OpKind::Conditional:{
            const auto& c=static_cast<const plan::Conditional&>(op); setPath("<if>");
            runtime::Value condition;
            if(!eval(c.condition.get(),env,condition,err)) return false;
            const auto* selected=std::get_if<bool>(&condition);
            if(!selected){err="conditional condition did not evaluate to boolean";return false;}
            return members(*selected?c.thenMembers:c.elseMembers,o,err);
        }
        case plan::OpKind::Callback:{
            const auto& cb=static_cast<const plan::Callback&>(op); setPath(cb.name);
            if(cb.direction != core::CallbackDirection::Encode){ err="callback direction is not valid for encoder: "+cb.name; return false; }
            if(!opt.callbacks){ err="encode callback registry is not configured: "+cb.name; return false; }
            CallbackArgs args;
            args.reserve(cb.args.size());
            for(const auto& expression : cb.args){
                CallbackValue value;
                if(!eval(expression.get(), env, value, err)) return false;
                args.push_back(value);
            }
            size_t callbackPos = w.pos();
            auto cr = opt.callbacks->call(cb.name, w.dataMutable(), callbackPos, callbackEnvironment(), args); if(!cr.success){ err=cr.error; return false; }
            w.setPos(callbackPos);
            return true;
        }
    }return false;}
    void addSizeProperties(const plan::Struct& s) {
        env[core::BuiltinMinSizeInBytesSymbolId] = runtime::Value(s.minSizeInBytes);
        if (s.sizeInBytes) env[core::BuiltinSizeInBytesSymbolId] = runtime::Value(*s.sizeInBytes);
        if (s.maxSizeInBytes) env[core::BuiltinMaxSizeInBytesSymbolId] = runtime::Value(*s.maxSizeInBytes);
    }
    bool writeStruct(const plan::Struct&s,const Value&v,std::string&err){
        auto o=std::get_if<Value::Object>(&v.data);
        if(!o){err="struct value must be an object: "+s.name;return false;}
        if(++recursionDepth > opt.maxRecursionDepth){ --recursionDepth; err="maximum recursion depth exceeded"; return false; }
        size_t start=w.pos();
        size_t old=base;
        auto saved=env;
                addSizeProperties(s);
                w.endian(rt(s.endian));
        bool ok=members(s.members,*o,err);
        if (ok) {
            for (const auto& requirement : s.requirements) {
                runtime::Value condition;
                if (!eval(requirement.get(), env, condition, err)) { ok=false; break; }
                const auto* passed = std::get_if<bool>(&condition);
                if (!passed || !*passed) { err="requires constraint failed"; ok=false; break; }
            }
        }
        if(!ok){
            // Keep the exact failure position in the diagnostic before rolling the writer back.
            // The rollback remains transactional, while diagnostics still identify the byte at
            // which the failing member was reached.
            const size_t failureOffset=w.pos();
            if(!diag) diag=diagnostics::make(diagnostics::classify(err,true),err,path,failureOffset);
            w.truncate(start);
            w.seek(start);
        }
        env=saved;
        base=old;
        --recursionDepth;
        return ok;
    }
public:
    const diagnostics::Diagnostic& diagnostic() const noexcept { return diag; }
    Encoder(const plan::Module&p_,const Options&o):p(p_),opt(o){}
    Result run(const plan::Struct&s,const Value&v){
        Result z; std::string err; env = opt.parameters;
        for (const auto& pmeta : p.parameters) {
            const auto it = env.find(pmeta.symbol);
            if (it == env.end()) { err="missing parameter: "+pmeta.name; z.error=err; return z; }
            const auto& n = pmeta.type.name;
            const bool ok = (n.size() > 1 && n[0] == 'u' && std::holds_alternative<uint64_t>(it->second)) ||
                            (n.size() > 1 && n[0] == 'i' && std::holds_alternative<int64_t>(it->second)) ||
                            ((n == "f32" || n == "f64") && std::holds_alternative<double>(it->second));
            if (!ok) { err="parameter type mismatch: "+pmeta.name; z.error=err; return z; }
            if (n[0] == 'u' && n.size() > 1) { const unsigned bits=static_cast<unsigned>(std::stoul(n.substr(1))); const auto v=std::get<uint64_t>(it->second); if (bits < 64 && v >= (uint64_t(1) << bits)) { err="parameter value out of range: "+pmeta.name; z.error=err; return z; } }
            if (n[0] == 'i' && n.size() > 1) { const unsigned bits=static_cast<unsigned>(std::stoul(n.substr(1))); const auto v=std::get<int64_t>(it->second); if (bits < 64) { const int64_t lo=-(int64_t(1) << (bits-1)), hi=(int64_t(1) << (bits-1))-1; if (v < lo || v > hi) { err="parameter value out of range: "+pmeta.name; z.error=err; return z; } } }
        }
        for (const auto& kv : env) if (!p.symbolTable.find(kv.first) || p.symbolTable.find(kv.first)->kind != core::SymbolKind::Parameter) { err="extra parameter SymbolId"; z.error=err; return z; }
        // Module constants and enum items are compile-time values materialized
        // in Plan. They are part of every executable struct environment; only
        // caller-supplied values are restricted to parameters above.
        for (const auto& c : p.constants) env[c.symbol] = c.value;
        for (const auto& e : p.enums) for (const auto& item : e.items) env[item.symbol] = item.value;
        path=s.name;
        if(!writeStruct(s,v,err)){
            z.error=err;
            if(!diag) diag=diagnostics::make(diagnostics::classify(err,true),err,path,w.pos());
            return z;
        }
        z.data=w.data(); z.success=true; return z;
    }
};
}
Engine::Engine(const plan::Module&plan,Options options):plan_(plan),options_(options){}
Result Engine::encode(const std::string&structName,const Value&value)const{
    auto sid=plan_.symbolTable.findId(structName);
    auto it=plan_.structIndexBySymbol.find(sid);
    if(sid==core::InvalidSymbolId || it==plan_.structIndexBySymbol.end()){Result z;z.error="unknown struct: "+structName;z.diagnostic=diagnostics::make(diagnostics::Code::UnknownStruct,z.error,structName,0);return z;}
    Encoder e(plan_,options_);Result z=e.run(plan_.structs[it->second],value);
    if(!z.success) { z.diagnostic=e.diagnostic(); if(!z.diagnostic) z.diagnostic=diagnostics::make(diagnostics::classify(z.error,true),z.error,structName,0); }
    return z;
}
} // namespace embx::encoder
