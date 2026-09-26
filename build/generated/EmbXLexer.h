
// Generated from C:/projects/embx/grammar/EmbX.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  EmbXLexer : public antlr4::Lexer {
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

  explicit EmbXLexer(antlr4::CharStream *input);

  ~EmbXLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

