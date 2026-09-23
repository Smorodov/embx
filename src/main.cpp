#include "parser/ParserDriver.h"
#include "compiler/Compiler.h"
#include <fstream>
#include <iostream>
namespace { using namespace embx::ast;
void ind(int n){while(n--)std::cout<<"  ";}
void ex(const Expr*e){if(!e)return; if(e->kind==ExprKind::Parenthesized){std::cout<<"(";ex(e->left.get());std::cout<<")";}else if(e->kind==ExprKind::Unary){std::cout<<e->op;ex(e->right.get());}else if(e->kind==ExprKind::Binary){ex(e->left.get());std::cout<<e->op;ex(e->right.get());}else std::cout<<e->text;}
void tr(const TypeRef&t){std::cout<<t.name;for(const auto&s:t.suffixes){std::cout<<"[";if(s.dynamic)std::cout<<"*";else ex(s.expr.get());std::cout<<"]";}}
void mem(const Member*m,int d){ind(d);if(auto*f=dynamic_cast<const Field*>(m)){std::cout<<"field "<<f->name<<": ";tr(f->type);if(f->bits)std::cout<<" ("<<f->bits<<")";if(!f->assertion.empty())std::cout<<" = "<<f->assertion;std::cout<<"\n";}else if(auto*b=dynamic_cast<const Bits*>(m)){std::cout<<"bits\n";for(auto&f:b->fields)mem(f.get(),d+1);}else if(auto*v=dynamic_cast<const Variant*>(m)){std::cout<<"variant "<<v->name<<" by ";ex(v->discriminator.get());std::cout<<"\n";for(auto&c:v->cases){ind(d+1);std::cout<<"case ";ex(c.tag.get());if(c.type){std::cout<<": ";tr(*c.type);std::cout<<"\n";}else{std::cout<<":\n";for(auto&x:c.members)mem(x.get(),d+2);}}if(v->defaultType){ind(d+1);std::cout<<"default: ";tr(*v->defaultType);std::cout<<"\n";}else for(auto&x:v->defaultMembers){ind(d+1);std::cout<<"default:\n";mem(x.get(),d+2);}}else if(auto*b=dynamic_cast<const Block*>(m)){std::cout<<"block "<<b->name<<"[";ex(b->size.get());std::cout<<"]\n";for(auto&x:b->members)mem(x.get(),d+1);}else if(auto*a=dynamic_cast<const At*>(m)){std::cout<<"at(";ex(a->offset.get());std::cout<<")\n";for(auto&x:a->members)mem(x.get(),d+1);}else if(auto*a=dynamic_cast<const Align*>(m)){std::cout<<"align(";ex(a->alignment.get());std::cout<<")\n";}else if(auto*c=dynamic_cast<const Callback*>(m))std::cout<<"callback "<<c->name<<"\n";}
void dump(const Module&m){std::cout<<"module\n";for(auto&a:m.aliases){ind(1);std::cout<<"type "<<a.name<<" = ";tr(a.target);std::cout<<"\n";}for(auto&c:m.constants){ind(1);std::cout<<(c.computed?"computed ":"const ")<<c.name<<" = ";ex(c.expr.get());std::cout<<"\n";}for(auto&e:m.enums){ind(1);std::cout<<"enum "<<e.name; if(e.underlying){std::cout<<": ";tr(*e.underlying);}std::cout<<"\n";for(auto&i:e.items){ind(2);std::cout<<i.name<<" = ";ex(i.value.get());std::cout<<"\n";}}for(auto&c:m.callbacks){ind(1);std::cout<<"callback "<<c.name;if(c.parameter)std::cout<<"("<<*c.parameter<<")";std::cout<<"\n";}for(auto&s:m.structs){ind(1);std::cout<<"struct "<<s->name<<"\n";for(auto&x:s->members)mem(x.get(),2);}}
}
int main(int argc,char**argv){
#ifdef EMBX_VERSION
    const std::string projectVersion = EMBX_VERSION;
#else
    const std::string projectVersion = "unknown";
#endif
const std::string version = projectVersion;
    auto usage=[&](){
        std::cout << "EmbX " << version << "\n"
                  << "Usage: embx <file.embx> [--dump-ast|--generate-cpp <prefix>]\n"
                  << "       embx --help\n"
                  << "       embx --version\n";
    };
    if(argc==1){usage();return 2;}
    if(std::string(argv[1])=="--help"){usage();return 0;}
    if(std::string(argv[1])=="--version"){std::cout << version << "\n";return 0;}
    if(std::string(argv[1]).rfind("--",0)==0){
        std::cerr << "unknown option: " << argv[1] << "\n";
        usage();
        return 2;
    }
    if(argc>4){std::cerr<<"too many arguments\n";usage();return 2;}
    std::string e;
    auto compilation=embx::compiler::compileFile(argv[1],e);
    if(!compilation){std::cerr<<"compile error: "<<e<<"\n";return 1;}
    auto& m=*compilation->ast;
    if(argc>2 && std::string(argv[2])=="--dump-ast") {
        if(argc!=3) { std::cerr << "--dump-ast takes no additional arguments\n"; usage(); return 2; }
        dump(m);
        return 0;
    }
    if(argc>2 && std::string(argv[2])=="--generate-cpp") {
        if(argc!=4 || std::string(argv[3]).empty()) { std::cerr << "--generate-cpp requires an output prefix\n"; usage(); return 2; }
        embx::codegen::Output out;
        std::string genError;
        if(!embx::codegen::generateCpp(*compilation->plan,out,genError)) { std::cerr << "codegen error: " << genError << "\n"; return 1; }
        const std::string prefix=argv[3];
        const auto slash=prefix.find_last_of("/\\");
        const std::string headerName=prefix.substr(slash==std::string::npos?0:slash+1)+".hpp";
        std::ofstream h(prefix+".hpp",std::ios::binary), c(prefix+".cpp",std::ios::binary);
        if(!h||!c){std::cerr<<"cannot open C++ generator output files for prefix: "<<prefix<<"\n";return 1;}
        h<<out.header;
        auto source=out.source;
        const auto marker=source.find("generated.hpp");
        if(marker!=std::string::npos) source.replace(marker,std::string("generated.hpp").size(),headerName);
        c<<source;
        std::cout<<"Generated: "<<prefix<<".hpp\nGenerated: "<<prefix<<".cpp\n";
        return 0;
    }
    if(argc>2){std::cerr<<"unknown option: "<<argv[2]<<"\n";usage();return 2;}
    std::cout<<"OK: parsed\n";
    return 0;
}