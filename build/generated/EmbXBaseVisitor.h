
// Generated from C:/projects/embx/grammar/EmbX.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "EmbXVisitor.h"


/**
 * This class provides an empty implementation of EmbXVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  EmbXBaseVisitor : public EmbXVisitor {
public:

  virtual std::any visitModule(EmbXParser::ModuleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNamespaceDecl(EmbXParser::NamespaceDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitImportDecl(EmbXParser::ImportDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitQualifiedName(EmbXParser::QualifiedNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitItem(EmbXParser::ItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEndianDirective(EmbXParser::EndianDirectiveContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConstDecl(EmbXParser::ConstDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitComputedDecl(EmbXParser::ComputedDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumDecl(EmbXParser::EnumDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumItem(EmbXParser::EnumItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeAlias(EmbXParser::TypeAliasContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStructDecl(EmbXParser::StructDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRequiresDecl(EmbXParser::RequiresDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMember(EmbXParser::MemberContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVirtualDecl(EmbXParser::VirtualDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAliasDecl(EmbXParser::AliasDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFieldDecl(EmbXParser::FieldDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFieldModifier(EmbXParser::FieldModifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTransform(EmbXParser::TransformContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBitsBlock(EmbXParser::BitsBlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBitField(EmbXParser::BitFieldContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConditionalDecl(EmbXParser::ConditionalDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVariantDecl(EmbXParser::VariantDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVariantCase(EmbXParser::VariantCaseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVariantDefault(EmbXParser::VariantDefaultContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVariantBody(EmbXParser::VariantBodyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlockDecl(EmbXParser::BlockDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAtDecl(EmbXParser::AtDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlignDecl(EmbXParser::AlignDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCallbackDecl(EmbXParser::CallbackDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParameterDecl(EmbXParser::ParameterDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCallbackUse(EmbXParser::CallbackUseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArgList(EmbXParser::ArgListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDocComment(EmbXParser::DocCommentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttributeDecl(EmbXParser::AttributeDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttributeType(EmbXParser::AttributeTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttributes(EmbXParser::AttributesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttributeEntry(EmbXParser::AttributeEntryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeRef(EmbXParser::TypeRefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTerminatedSequenceSuffix(EmbXParser::TerminatedSequenceSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeSuffix(EmbXParser::TypeSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBaseType(EmbXParser::BaseTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimitive(EmbXParser::PrimitiveContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpr(EmbXParser::ExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimary(EmbXParser::PrimaryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLiteral(EmbXParser::LiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEndian(EmbXParser::EndianContext *ctx) override {
    return visitChildren(ctx);
  }


};

