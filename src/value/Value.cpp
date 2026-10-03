#include "value/Value.h"
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

namespace embx::value {
namespace {
void append(const Value& v, std::ostringstream& os) {
    switch (v.kind()) {
    case Value::Kind::Null: os << "null"; break;
    case Value::Kind::Signed: os << std::get<int64_t>(v.data); break;
    case Value::Kind::Unsigned: os << std::get<uint64_t>(v.data); break;
    case Value::Kind::Floating: {
        os << std::setprecision(17) << std::get<double>(v.data); break;
    }
    case Value::Kind::Boolean: os << (std::get<bool>(v.data) ? "true" : "false"); break;
    case Value::Kind::String: os << std::quoted(std::get<std::string>(v.data)); break;
    case Value::Kind::Bytes: {
        os << "0x" << std::hex << std::setfill('0');
        for (auto b : std::get<Value::Bytes>(v.data)) os << std::setw(2) << unsigned(b);
        os << std::dec; break;
    }
    case Value::Kind::Array: {
        os << '['; bool first=true;
        for (const auto& x : std::get<Value::Array>(v.data)) { if (!first) os << ','; first=false; append(x, os); }
        os << ']'; break;
    }
    case Value::Kind::Object: {
        const auto& o=std::get<Value::Object>(v.data);
        std::vector<std::string> keys; keys.reserve(o.size());
        for (const auto& kv:o) keys.push_back(kv.first);
        std::sort(keys.begin(), keys.end());
        os << '{'; bool first=true;
        for (const auto& k:keys) { if(!first) os<<','; first=false; os<<std::quoted(k)<<':'; append(o.at(k),os); }
        os << '}'; break;
    }
    }
}
bool eq(const Value& a,const Value& b) noexcept {
    if (a.data.index()!=b.data.index()) return false;
    if (auto p=std::get_if<Value::Object>(&a.data)) {
        const auto& q=std::get<Value::Object>(b.data); if(p->size()!=q.size()) return false;
        for(const auto& kv:*p){auto it=q.find(kv.first);if(it==q.end()||!eq(kv.second,it->second))return false;} return true;
    }
    if (auto p=std::get_if<Value::Array>(&a.data)) {const auto&q=std::get<Value::Array>(b.data);if(p->size()!=q.size())return false;for(size_t i=0;i<p->size();++i)if(!eq((*p)[i],q[i]))return false;return true;}
    switch(a.kind()){
    case Value::Kind::Null:return true;
    case Value::Kind::Signed:return std::get<int64_t>(a.data)==std::get<int64_t>(b.data);
    case Value::Kind::Unsigned:return std::get<uint64_t>(a.data)==std::get<uint64_t>(b.data);
    case Value::Kind::Floating:return std::get<double>(a.data)==std::get<double>(b.data);
    case Value::Kind::Boolean:return std::get<bool>(a.data)==std::get<bool>(b.data);
    case Value::Kind::String:return std::get<std::string>(a.data)==std::get<std::string>(b.data);
    case Value::Kind::Bytes:return std::get<Value::Bytes>(a.data)==std::get<Value::Bytes>(b.data);
    case Value::Kind::Object:case Value::Kind::Array:break;
    }
    return false;
}
}
Value::Kind Value::kind() const noexcept {
    switch(data.index()){case 0:return Kind::Null;case 1:return Kind::Signed;case 2:return Kind::Unsigned;case 3:return Kind::Floating;case 4:return Kind::Boolean;case 5:return Kind::String;case 6:return Kind::Bytes;case 7:return Kind::Object;default:return Kind::Array;}
}
const char* Value::typeName() const noexcept {
    switch(kind()){case Kind::Null:return "null";case Kind::Signed:return "signed";case Kind::Unsigned:return "unsigned";case Kind::Floating:return "floating";case Kind::Boolean:return "boolean";case Kind::String:return "string";case Kind::Bytes:return "bytes";case Kind::Object:return "object";case Kind::Array:return "array";} return "unknown";
}
bool Value::isNumber() const noexcept { return std::holds_alternative<int64_t>(data)||std::holds_alternative<uint64_t>(data)||std::holds_alternative<double>(data); }
const Value* Value::get(const std::string& key) const noexcept {auto p=std::get_if<Object>(&data);if(!p)return nullptr;auto it=p->find(key);return it==p->end()?nullptr:&it->second;}
Value* Value::get(const std::string& key) noexcept {auto p=std::get_if<Object>(&data);if(!p)return nullptr;auto it=p->find(key);return it==p->end()?nullptr:&it->second;}
const Value& Value::at(const std::string& key) const {auto p=get(key);if(!p)throw std::out_of_range("missing value field: "+key);return *p;}
Value& Value::at(const std::string& key) {auto p=get(key);if(!p)throw std::out_of_range("missing value field: "+key);return *p;}
std::string Value::debugString() const {std::ostringstream os;append(*this,os);return os.str();}
bool equal(const Value&a,const Value&b) noexcept{return eq(a,b);}
} // namespace embx::value
