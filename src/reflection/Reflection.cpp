#include "reflection/Reflection.h"
namespace embx::reflection {
namespace {
std::string valueText(const runtime::Value& v) {
    if (const auto* p = std::get_if<std::int64_t>(&v)) return std::to_string(*p);
    if (const auto* p = std::get_if<std::uint64_t>(&v)) return std::to_string(*p);
    return {};
}

std::vector<std::string> attrs(const std::vector<core::Attribute>& a) {
    std::vector<std::string> out;
    for (const auto& x : a) out.push_back(x.hasValue ? x.name + "=" + x.value : x.name);
    return out;
}
LayoutClass layoutClass(const std::optional<runtime::LayoutSize>& max, runtime::LayoutSize min) {
    if (!max) return LayoutClass::Unbounded;
    return *max == min ? LayoutClass::Exact : LayoutClass::Bounded;
}

void setExact(Member& m, runtime::LayoutSize size) {
    m.layoutClass = LayoutClass::Exact;
    m.minSize = size;
    m.maxSize = size;
}

void add(const plan::Op& op, std::vector<Member>& out){
    Member m{}; m.attributes=attrs(op.attributes); m.documentation=op.documentation;
    switch(op.kind){
    case plan::OpKind::Virtual:{auto&x=static_cast<const plan::Virtual&>(op);m.kind=Kind::Virtual;m.name=x.name;m.type="expression";m.symbol=x.symbol;if(x.expression)m.children.push_back(x.expression->text);break;}
    case plan::OpKind::Alias:{auto&x=static_cast<const plan::FieldAlias&>(op);m.kind=Kind::Alias;m.name=x.name;m.type="alias";m.symbol=x.symbol;m.targetSymbol=x.target;m.children.push_back(x.targetName);break;}
    case plan::OpKind::Field:{auto&x=static_cast<const plan::Field&>(op);m.kind=Kind::Field;m.name=x.name;m.type=x.type.name;m.symbol=x.symbol;m.staticSize=x.staticSize;m.staticOffset=x.staticOffset;m.bits=x.bits;m.terminated=!x.type.terminator.empty();m.terminator=x.type.terminator;m.maxPayload=x.type.maxPayload;if(x.staticSize)setExact(m,*x.staticSize);else if(m.terminated){const auto term=x.type.terminator.size();const auto maxPayload=x.type.maxPayload.value_or(0);m.minSize=static_cast<runtime::LayoutSize>(term);m.maxSize=static_cast<runtime::LayoutSize>(maxPayload)+static_cast<runtime::LayoutSize>(term);m.layoutClass=LayoutClass::Bounded;}else if(core::hasRemaining(x.type)){m.layoutClass=LayoutClass::Unbounded;}if(x.transform){m.transform=x.transform->name;m.transformFactor=x.transform->factor;}break;}
    case plan::OpKind::Bits:{auto&x=static_cast<const plan::Bits&>(op);m.kind=Kind::Bits;m.name="bits";m.staticSize=x.storageBytes;setExact(m,x.storageBytes);for(const auto&f:x.fields){m.children.push_back(f.name);Member bm{};bm.kind=Kind::Field;bm.name=f.name;bm.type=f.type.name;bm.bits=f.bits;bm.documentation=f.documentation;bm.attributes=attrs(f.attributes);bm.symbol=f.symbol;m.nested.push_back(std::move(bm));}break;}
    case plan::OpKind::Variant:{auto&x=static_cast<const plan::Variant&>(op);m.kind=Kind::Variant;m.name=x.name;m.discriminator=x.discriminator?x.discriminator->text:std::string{};for(const auto&c:x.cases)m.children.push_back(valueText(c.tag));if(x.hasDefault)m.children.push_back("default");break;}
    case plan::OpKind::Conditional:{auto&x=static_cast<const plan::Conditional&>(op);m.kind=Kind::Conditional;m.name="if";if(x.condition)m.children.push_back(x.condition->text);for(auto&c:x.thenMembers)if(c)add(*c,m.nested);for(auto&c:x.elseMembers)if(c)add(*c,m.nested);out.push_back(std::move(m));return;}
    case plan::OpKind::Block:{auto&x=static_cast<const plan::Block&>(op);m.kind=Kind::Block;m.name=x.name.empty()?"block":x.name;m.staticSize=x.staticSize;if(x.staticSize)setExact(m,*x.staticSize);for(auto&c:x.members)if(c)add(*c,m.nested);out.push_back(std::move(m));return;}
    case plan::OpKind::At:{auto&x=static_cast<const plan::At&>(op);m.kind=Kind::At;m.name="at";m.staticOffset=x.staticOffset;for(auto&c:x.members)if(c)add(*c,m.nested);out.push_back(std::move(m));return;}
    case plan::OpKind::Align:{auto&x=static_cast<const plan::Align&>(op);m.kind=Kind::Align;m.name="align";m.staticSize=x.staticAlignment;if(x.staticAlignment)setExact(m,*x.staticAlignment);break;}
    case plan::OpKind::Callback:{auto&x=static_cast<const plan::Callback&>(op);m.kind=Kind::Callback;m.name=x.name;m.callbackDirection=x.direction;for(const auto& e:x.args)if(e)m.callbackArgs.push_back(e->text);break;}
    }
    out.push_back(std::move(m));
}
}
Module inspect(const plan::Module&p){
    Module m; m.plan=&p; m.documentation=p.documentation; for (const auto& d : p.attributeDefinitions) m.attributeDefinitions.push_back({d.name, d.type});
    for(auto&x:p.structs){m.structNames.push_back(x.name);m.structAttributes.push_back(attrs(x.attributes));}
    for(auto&x:p.aliases){m.aliasNames.push_back(x.name);m.aliasAttributes.push_back(attrs(x.attributes));}
    for(auto&x:p.enums){m.enumNames.push_back(x.name);m.enumAttributes.push_back(attrs(x.attributes));std::vector<EnumItem> items;for(const auto& item:x.items)items.push_back({item.name,item.documentation});m.enumItems.push_back(std::move(items));}
    for(auto&x:p.constants){m.constantNames.push_back(x.name);m.constantAttributes.push_back(attrs(x.attributes));}
    for(auto&x:p.callbacks){m.callbackNames.push_back(x.name);m.callbackAttributes.push_back(attrs(x.attributes));m.callbackDirections.push_back(x.direction);}
    for(auto&x:p.parameters)m.parameters.push_back({x.name,x.symbol,x.type.name,x.documentation});
    return m;
}
std::optional<Struct> inspectStruct(const plan::Module&p,const std::string&name){auto sid=p.symbolTable.findId(name);if(sid==core::InvalidSymbolId)return std::nullopt;auto it=p.structIndexBySymbol.find(sid);if(it==p.structIndexBySymbol.end())return std::nullopt;const auto&s=p.structs[it->second];Struct r;r.attributes=attrs(s.attributes);r.documentation=s.documentation;r.name=s.name;r.endian=s.endian;r.staticSize=s.staticSize;r.minSize=s.minSizeInBytes;r.maxSize=s.maxSizeInBytes;r.layoutClass=layoutClass(s.maxSizeInBytes,s.minSizeInBytes);for(const auto& req:s.requirements)if(req)r.requirements.push_back(req->text);for(auto&op:s.members)if(op)add(*op,r.members);return r;}
const char* kindName(Kind k)noexcept{switch(k){case Kind::Field:return"field";case Kind::Virtual:return"virtual";case Kind::Alias:return"alias";case Kind::Bits:return"bits";case Kind::Variant:return"variant";case Kind::Conditional:return"conditional";case Kind::Block:return"block";case Kind::At:return"at";case Kind::Align:return"align";case Kind::Callback:return"callback";}return"unknown";}
}
