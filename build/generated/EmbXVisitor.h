
// Generated from C:/projects/embx/grammar/EmbX.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "EmbXParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by EmbXParser.
 */
class  EmbXVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by EmbXParser.
   */
    virtual std::any visitModule(EmbXParser::ModuleContext *context) = 0;

    virtual std::any visitNamespaceDecl(EmbXParser::NamespaceDeclContext *context) = 0;

    virtual std::any visitImportDecl(EmbXParser::ImportDeclContext *context) = 0;

    virtual std::any visitQualifiedName(EmbXParser::QualifiedNameContext *context) = 0;

    virtual std::any visitItem(EmbXParser::ItemContext *context) = 0;

    virtual std::any visitEndianDirective(EmbXParser::EndianDirectiveContext *context) = 0;

    virtual std::any visitConstDecl(EmbXParser::ConstDeclContext *context) = 0;

    virtual std::any visitComputedDecl(EmbXParser::ComputedDeclContext *context) = 0;

    virtual std::any visitEnumDecl(EmbXParser::EnumDeclContext *context) = 0;

    virtual std::any visitEnumItem(EmbXParser::EnumItemContext *context) = 0;

    virtual std::any visitTypeAlias(EmbXParser::TypeAliasContext *context) = 0;

    virtual std::any visitStructDecl(EmbXParser::StructDeclContext *context) = 0;

    virtual std::any visitRequiresDecl(EmbXParser::RequiresDeclContext *context) = 0;

    virtual std::any visitMember(EmbXParser::MemberContext *context) = 0;

    virtual std::any visitVirtualDecl(EmbXParser::VirtualDeclContext *context) = 0;

    virtual std::any visitAliasDecl(EmbXParser::AliasDeclContext *context) = 0;

    virtual std::any visitFieldDecl(EmbXParser::FieldDeclContext *context) = 0;

    virtual std::any visitFieldModifier(EmbXParser::FieldModifierContext *context) = 0;

    virtual std::any visitTransform(EmbXParser::TransformContext *context) = 0;

    virtual std::any visitBitsBlock(EmbXParser::BitsBlockContext *context) = 0;

    virtual std::any visitBitField(EmbXParser::BitFieldContext *context) = 0;

    virtual std::any visitConditionalDecl(EmbXParser::ConditionalDeclContext *context) = 0;

    virtual std::any visitVariantDecl(EmbXParser::VariantDeclContext *context) = 0;

    virtual std::any visitVariantCase(EmbXParser::VariantCaseContext *context) = 0;

    virtual std::any visitVariantDefault(EmbXParser::VariantDefaultContext *context) = 0;

    virtual std::any visitVariantBody(EmbXParser::VariantBodyContext *context) = 0;

    virtual std::any visitBlockDecl(EmbXParser::BlockDeclContext *context) = 0;

    virtual std::any visitAtDecl(EmbXParser::AtDeclContext *context) = 0;

    virtual std::any visitAlignDecl(EmbXParser::AlignDeclContext *context) = 0;

    virtual std::any visitCallbackDecl(EmbXParser::CallbackDeclContext *context) = 0;

    virtual std::any visitParameterDecl(EmbXParser::ParameterDeclContext *context) = 0;

    virtual std::any visitCallbackUse(EmbXParser::CallbackUseContext *context) = 0;

    virtual std::any visitArgList(EmbXParser::ArgListContext *context) = 0;

    virtual std::any visitDocComment(EmbXParser::DocCommentContext *context) = 0;

    virtual std::any visitAttributeDecl(EmbXParser::AttributeDeclContext *context) = 0;

    virtual std::any visitAttributeType(EmbXParser::AttributeTypeContext *context) = 0;

    virtual std::any visitAttributes(EmbXParser::AttributesContext *context) = 0;

    virtual std::any visitAttributeEntry(EmbXParser::AttributeEntryContext *context) = 0;

    virtual std::any visitTypeRef(EmbXParser::TypeRefContext *context) = 0;

    virtual std::any visitTerminatedSequenceSuffix(EmbXParser::TerminatedSequenceSuffixContext *context) = 0;

    virtual std::any visitTypeSuffix(EmbXParser::TypeSuffixContext *context) = 0;

    virtual std::any visitBaseType(EmbXParser::BaseTypeContext *context) = 0;

    virtual std::any visitPrimitive(EmbXParser::PrimitiveContext *context) = 0;

    virtual std::any visitExpr(EmbXParser::ExprContext *context) = 0;

    virtual std::any visitPrimary(EmbXParser::PrimaryContext *context) = 0;

    virtual std::any visitLiteral(EmbXParser::LiteralContext *context) = 0;

    virtual std::any visitEndian(EmbXParser::EndianContext *context) = 0;


};

