
// Generated from C:/projects/embx/grammar/EmbX.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  EmbXParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, STAR = 23, UNTIL = 24, MAX = 25, SCOPE = 26, 
    AT = 27, ATTRIBUTE = 28, REQUIRES = 29, ENDIAN = 30, CONST = 31, COMPUTED = 32, 
    ENUM = 33, STRUCT = 34, TYPE = 35, BITS = 36, VARIANT = 37, BY = 38, 
    DEFAULT = 39, BLOCK = 40, ATCALL = 41, ALIGN = 42, CALLBACK = 43, PARAM = 44, 
    IF = 45, ELSE = 46, LET = 47, NAMESPACE = 48, IMPORT = 49, AS = 50, 
    ALIAS = 51, TRANSFORM = 52, NEXT = 53, TRUE = 54, FALSE = 55, SIZE_IN_BYTES = 56, 
    MIN_SIZE_IN_BYTES = 57, MAX_SIZE_IN_BYTES = 58, LITTLE = 59, BIG = 60, 
    NATIVE = 61, U8 = 62, I8 = 63, U16 = 64, I16 = 65, U32 = 66, I32 = 67, 
    U64 = 68, I64 = 69, F32 = 70, F64 = 71, BYTES = 72, STRINGTYPE = 73, 
    ID = 74, HEX = 75, FLOAT = 76, INT = 77, STRING = 78, WS = 79, DOC_COMMENT = 80, 
    LINECOMMENT = 81, BLOCKCOMMENT = 82
  };

  enum {
    RuleModule = 0, RuleNamespaceDecl = 1, RuleImportDecl = 2, RuleQualifiedName = 3, 
    RuleItem = 4, RuleEndianDirective = 5, RuleConstDecl = 6, RuleComputedDecl = 7, 
    RuleEnumDecl = 8, RuleEnumItem = 9, RuleTypeAlias = 10, RuleStructDecl = 11, 
    RuleRequiresDecl = 12, RuleMember = 13, RuleVirtualDecl = 14, RuleAliasDecl = 15, 
    RuleFieldDecl = 16, RuleFieldModifier = 17, RuleTransform = 18, RuleBitsBlock = 19, 
    RuleBitField = 20, RuleConditionalDecl = 21, RuleVariantDecl = 22, RuleVariantCase = 23, 
    RuleVariantDefault = 24, RuleVariantBody = 25, RuleBlockDecl = 26, RuleAtDecl = 27, 
    RuleAlignDecl = 28, RuleCallbackDecl = 29, RuleParameterDecl = 30, RuleCallbackUse = 31, 
    RuleArgList = 32, RuleDocComment = 33, RuleAttributeDecl = 34, RuleAttributeType = 35, 
    RuleAttributes = 36, RuleAttributeEntry = 37, RuleTypeRef = 38, RuleTerminatedSequenceSuffix = 39, 
    RuleTypeSuffix = 40, RuleBaseType = 41, RulePrimitive = 42, RuleExpr = 43, 
    RulePrimary = 44, RuleLiteral = 45, RuleEndian = 46
  };

  explicit EmbXParser(antlr4::TokenStream *input);

  EmbXParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~EmbXParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class ModuleContext;
  class NamespaceDeclContext;
  class ImportDeclContext;
  class QualifiedNameContext;
  class ItemContext;
  class EndianDirectiveContext;
  class ConstDeclContext;
  class ComputedDeclContext;
  class EnumDeclContext;
  class EnumItemContext;
  class TypeAliasContext;
  class StructDeclContext;
  class RequiresDeclContext;
  class MemberContext;
  class VirtualDeclContext;
  class AliasDeclContext;
  class FieldDeclContext;
  class FieldModifierContext;
  class TransformContext;
  class BitsBlockContext;
  class BitFieldContext;
  class ConditionalDeclContext;
  class VariantDeclContext;
  class VariantCaseContext;
  class VariantDefaultContext;
  class VariantBodyContext;
  class BlockDeclContext;
  class AtDeclContext;
  class AlignDeclContext;
  class CallbackDeclContext;
  class ParameterDeclContext;
  class CallbackUseContext;
  class ArgListContext;
  class DocCommentContext;
  class AttributeDeclContext;
  class AttributeTypeContext;
  class AttributesContext;
  class AttributeEntryContext;
  class TypeRefContext;
  class TerminatedSequenceSuffixContext;
  class TypeSuffixContext;
  class BaseTypeContext;
  class PrimitiveContext;
  class ExprContext;
  class PrimaryContext;
  class LiteralContext;
  class EndianContext; 

  class  ModuleContext : public antlr4::ParserRuleContext {
  public:
    ModuleContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    antlr4::tree::TerminalNode *DOC_COMMENT();
    NamespaceDeclContext *namespaceDecl();
    std::vector<ImportDeclContext *> importDecl();
    ImportDeclContext* importDecl(size_t i);
    std::vector<ItemContext *> item();
    ItemContext* item(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModuleContext* module();

  class  NamespaceDeclContext : public antlr4::ParserRuleContext {
  public:
    NamespaceDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NAMESPACE();
    QualifiedNameContext *qualifiedName();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NamespaceDeclContext* namespaceDecl();

  class  ImportDeclContext : public antlr4::ParserRuleContext {
  public:
    ImportDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IMPORT();
    antlr4::tree::TerminalNode *STRING();
    antlr4::tree::TerminalNode *AS();
    antlr4::tree::TerminalNode *ID();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ImportDeclContext* importDecl();

  class  QualifiedNameContext : public antlr4::ParserRuleContext {
  public:
    QualifiedNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SCOPE();
    antlr4::tree::TerminalNode* SCOPE(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  QualifiedNameContext* qualifiedName();

  class  ItemContext : public antlr4::ParserRuleContext {
  public:
    ItemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AttributeDeclContext *attributeDecl();
    EndianDirectiveContext *endianDirective();
    ConstDeclContext *constDecl();
    ComputedDeclContext *computedDecl();
    EnumDeclContext *enumDecl();
    StructDeclContext *structDecl();
    CallbackDeclContext *callbackDecl();
    ParameterDeclContext *parameterDecl();
    TypeAliasContext *typeAlias();
    DocCommentContext *docComment();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ItemContext* item();

  class  EndianDirectiveContext : public antlr4::ParserRuleContext {
  public:
    EndianDirectiveContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *AT();
    antlr4::tree::TerminalNode *ENDIAN();
    EndianContext *endian();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EndianDirectiveContext* endianDirective();

  class  ConstDeclContext : public antlr4::ParserRuleContext {
  public:
    ConstDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CONST();
    antlr4::tree::TerminalNode *ID();
    ExprContext *expr();
    AttributesContext *attributes();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstDeclContext* constDecl();

  class  ComputedDeclContext : public antlr4::ParserRuleContext {
  public:
    ComputedDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COMPUTED();
    antlr4::tree::TerminalNode *ID();
    ExprContext *expr();
    AttributesContext *attributes();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ComputedDeclContext* computedDecl();

  class  EnumDeclContext : public antlr4::ParserRuleContext {
  public:
    EnumDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ENUM();
    antlr4::tree::TerminalNode *ID();
    std::vector<EnumItemContext *> enumItem();
    EnumItemContext* enumItem(size_t i);
    AttributesContext *attributes();
    TypeRefContext *typeRef();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EnumDeclContext* enumDecl();

  class  EnumItemContext : public antlr4::ParserRuleContext {
  public:
    EnumItemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    ExprContext *expr();
    DocCommentContext *docComment();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EnumItemContext* enumItem();

  class  TypeAliasContext : public antlr4::ParserRuleContext {
  public:
    TypeAliasContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TYPE();
    antlr4::tree::TerminalNode *ID();
    TypeRefContext *typeRef();
    AttributesContext *attributes();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeAliasContext* typeAlias();

  class  StructDeclContext : public antlr4::ParserRuleContext {
  public:
    StructDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STRUCT();
    antlr4::tree::TerminalNode *ID();
    AttributesContext *attributes();
    EndianContext *endian();
    std::vector<RequiresDeclContext *> requiresDecl();
    RequiresDeclContext* requiresDecl(size_t i);
    std::vector<MemberContext *> member();
    MemberContext* member(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StructDeclContext* structDecl();

  class  RequiresDeclContext : public antlr4::ParserRuleContext {
  public:
    RequiresDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *REQUIRES();
    ExprContext *expr();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  RequiresDeclContext* requiresDecl();

  class  MemberContext : public antlr4::ParserRuleContext {
  public:
    MemberContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    FieldDeclContext *fieldDecl();
    VirtualDeclContext *virtualDecl();
    AliasDeclContext *aliasDecl();
    BitsBlockContext *bitsBlock();
    VariantDeclContext *variantDecl();
    ConditionalDeclContext *conditionalDecl();
    BlockDeclContext *blockDecl();
    AtDeclContext *atDecl();
    AlignDeclContext *alignDecl();
    CallbackUseContext *callbackUse();
    DocCommentContext *docComment();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MemberContext* member();

  class  VirtualDeclContext : public antlr4::ParserRuleContext {
  public:
    VirtualDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LET();
    antlr4::tree::TerminalNode *ID();
    ExprContext *expr();
    AttributesContext *attributes();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VirtualDeclContext* virtualDecl();

  class  AliasDeclContext : public antlr4::ParserRuleContext {
  public:
    AliasDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ALIAS();
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    AttributesContext *attributes();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AliasDeclContext* aliasDecl();

  class  FieldDeclContext : public antlr4::ParserRuleContext {
  public:
    FieldDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    TypeRefContext *typeRef();
    AttributesContext *attributes();
    std::vector<FieldModifierContext *> fieldModifier();
    FieldModifierContext* fieldModifier(size_t i);
    TransformContext *transform();
    LiteralContext *literal();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FieldDeclContext* fieldDecl();

  class  FieldModifierContext : public antlr4::ParserRuleContext {
  public:
    FieldModifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    EndianContext *endian();
    ExprContext *expr();
    antlr4::tree::TerminalNode *INT();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FieldModifierContext* fieldModifier();

  class  TransformContext : public antlr4::ParserRuleContext {
  public:
    TransformContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TRANSFORM();
    antlr4::tree::TerminalNode *ID();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TransformContext* transform();

  class  BitsBlockContext : public antlr4::ParserRuleContext {
  public:
    BitsBlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *BITS();
    AttributesContext *attributes();
    std::vector<BitFieldContext *> bitField();
    BitFieldContext* bitField(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BitsBlockContext* bitsBlock();

  class  BitFieldContext : public antlr4::ParserRuleContext {
  public:
    BitFieldContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    TypeRefContext *typeRef();
    antlr4::tree::TerminalNode *INT();
    DocCommentContext *docComment();
    std::vector<FieldModifierContext *> fieldModifier();
    FieldModifierContext* fieldModifier(size_t i);
    LiteralContext *literal();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BitFieldContext* bitField();

  class  ConditionalDeclContext : public antlr4::ParserRuleContext {
  public:
    ConditionalDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IF();
    ExprContext *expr();
    AttributesContext *attributes();
    std::vector<MemberContext *> member();
    MemberContext* member(size_t i);
    antlr4::tree::TerminalNode *ELSE();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConditionalDeclContext* conditionalDecl();

  class  VariantDeclContext : public antlr4::ParserRuleContext {
  public:
    VariantDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *VARIANT();
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *BY();
    ExprContext *expr();
    AttributesContext *attributes();
    std::vector<VariantCaseContext *> variantCase();
    VariantCaseContext* variantCase(size_t i);
    VariantDefaultContext *variantDefault();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VariantDeclContext* variantDecl();

  class  VariantCaseContext : public antlr4::ParserRuleContext {
  public:
    VariantCaseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExprContext *expr();
    VariantBodyContext *variantBody();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VariantCaseContext* variantCase();

  class  VariantDefaultContext : public antlr4::ParserRuleContext {
  public:
    VariantDefaultContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DEFAULT();
    VariantBodyContext *variantBody();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VariantDefaultContext* variantDefault();

  class  VariantBodyContext : public antlr4::ParserRuleContext {
  public:
    VariantBodyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TypeRefContext *typeRef();
    std::vector<MemberContext *> member();
    MemberContext* member(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VariantBodyContext* variantBody();

  class  BlockDeclContext : public antlr4::ParserRuleContext {
  public:
    BlockDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *BLOCK();
    ExprContext *expr();
    AttributesContext *attributes();
    antlr4::tree::TerminalNode *ID();
    std::vector<MemberContext *> member();
    MemberContext* member(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BlockDeclContext* blockDecl();

  class  AtDeclContext : public antlr4::ParserRuleContext {
  public:
    AtDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ATCALL();
    ExprContext *expr();
    AttributesContext *attributes();
    std::vector<MemberContext *> member();
    MemberContext* member(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AtDeclContext* atDecl();

  class  AlignDeclContext : public antlr4::ParserRuleContext {
  public:
    AlignDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ALIGN();
    ExprContext *expr();
    AttributesContext *attributes();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AlignDeclContext* alignDecl();

  class  CallbackDeclContext : public antlr4::ParserRuleContext {
  public:
    CallbackDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CALLBACK();
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    AttributesContext *attributes();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CallbackDeclContext* callbackDecl();

  class  ParameterDeclContext : public antlr4::ParserRuleContext {
  public:
    ParameterDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *PARAM();
    antlr4::tree::TerminalNode *ID();
    TypeRefContext *typeRef();
    AttributesContext *attributes();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParameterDeclContext* parameterDecl();

  class  CallbackUseContext : public antlr4::ParserRuleContext {
  public:
    CallbackUseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CALLBACK();
    antlr4::tree::TerminalNode *ID();
    AttributesContext *attributes();
    ArgListContext *argList();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CallbackUseContext* callbackUse();

  class  ArgListContext : public antlr4::ParserRuleContext {
  public:
    ArgListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ArgListContext* argList();

  class  DocCommentContext : public antlr4::ParserRuleContext {
  public:
    DocCommentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> DOC_COMMENT();
    antlr4::tree::TerminalNode* DOC_COMMENT(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DocCommentContext* docComment();

  class  AttributeDeclContext : public antlr4::ParserRuleContext {
  public:
    AttributeDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ATTRIBUTE();
    antlr4::tree::TerminalNode *ID();
    AttributeTypeContext *attributeType();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttributeDeclContext* attributeDecl();

  class  AttributeTypeContext : public antlr4::ParserRuleContext {
  public:
    AttributeTypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *U8();
    antlr4::tree::TerminalNode *I8();
    antlr4::tree::TerminalNode *U16();
    antlr4::tree::TerminalNode *I16();
    antlr4::tree::TerminalNode *U32();
    antlr4::tree::TerminalNode *I32();
    antlr4::tree::TerminalNode *U64();
    antlr4::tree::TerminalNode *I64();
    antlr4::tree::TerminalNode *F32();
    antlr4::tree::TerminalNode *F64();
    antlr4::tree::TerminalNode *STRINGTYPE();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttributeTypeContext* attributeType();

  class  AttributesContext : public antlr4::ParserRuleContext {
  public:
    AttributesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<AttributeEntryContext *> attributeEntry();
    AttributeEntryContext* attributeEntry(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttributesContext* attributes();

  class  AttributeEntryContext : public antlr4::ParserRuleContext {
  public:
    AttributeEntryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    LiteralContext *literal();
    antlr4::tree::TerminalNode *TRUE();
    antlr4::tree::TerminalNode *FALSE();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttributeEntryContext* attributeEntry();

  class  TypeRefContext : public antlr4::ParserRuleContext {
  public:
    TypeRefContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    QualifiedNameContext *qualifiedName();
    BaseTypeContext *baseType();
    TypeSuffixContext *typeSuffix();
    TerminatedSequenceSuffixContext *terminatedSequenceSuffix();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeRefContext* typeRef();

  class  TerminatedSequenceSuffixContext : public antlr4::ParserRuleContext {
  public:
    TerminatedSequenceSuffixContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *UNTIL();
    antlr4::tree::TerminalNode *MAX();
    antlr4::tree::TerminalNode *INT();
    std::vector<antlr4::tree::TerminalNode *> HEX();
    antlr4::tree::TerminalNode* HEX(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TerminatedSequenceSuffixContext* terminatedSequenceSuffix();

  class  TypeSuffixContext : public antlr4::ParserRuleContext {
  public:
    TypeSuffixContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STAR();
    ExprContext *expr();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeSuffixContext* typeSuffix();

  class  BaseTypeContext : public antlr4::ParserRuleContext {
  public:
    BaseTypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PrimitiveContext *primitive();
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *BYTES();
    antlr4::tree::TerminalNode *STRINGTYPE();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BaseTypeContext* baseType();

  class  PrimitiveContext : public antlr4::ParserRuleContext {
  public:
    PrimitiveContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *U8();
    antlr4::tree::TerminalNode *I8();
    antlr4::tree::TerminalNode *U16();
    antlr4::tree::TerminalNode *I16();
    antlr4::tree::TerminalNode *U32();
    antlr4::tree::TerminalNode *I32();
    antlr4::tree::TerminalNode *U64();
    antlr4::tree::TerminalNode *I64();
    antlr4::tree::TerminalNode *F32();
    antlr4::tree::TerminalNode *F64();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrimitiveContext* primitive();

  class  ExprContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *op = nullptr;
    ExprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PrimaryContext *primary();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    antlr4::tree::TerminalNode *STAR();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExprContext* expr();
  ExprContext* expr(int precedence);
  class  PrimaryContext : public antlr4::ParserRuleContext {
  public:
    PrimaryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INT();
    antlr4::tree::TerminalNode *HEX();
    antlr4::tree::TerminalNode *FLOAT();
    antlr4::tree::TerminalNode *STRING();
    antlr4::tree::TerminalNode *TRUE();
    antlr4::tree::TerminalNode *FALSE();
    antlr4::tree::TerminalNode *NEXT();
    antlr4::tree::TerminalNode *SIZE_IN_BYTES();
    antlr4::tree::TerminalNode *MIN_SIZE_IN_BYTES();
    antlr4::tree::TerminalNode *MAX_SIZE_IN_BYTES();
    QualifiedNameContext *qualifiedName();
    ExprContext *expr();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrimaryContext* primary();

  class  LiteralContext : public antlr4::ParserRuleContext {
  public:
    LiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INT();
    antlr4::tree::TerminalNode *HEX();
    antlr4::tree::TerminalNode *FLOAT();
    antlr4::tree::TerminalNode *STRING();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LiteralContext* literal();

  class  EndianContext : public antlr4::ParserRuleContext {
  public:
    EndianContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LITTLE();
    antlr4::tree::TerminalNode *BIG();
    antlr4::tree::TerminalNode *NATIVE();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EndianContext* endian();


  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool exprSempred(ExprContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

