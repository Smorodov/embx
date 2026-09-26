
// Generated from C:/projects/embx/grammar/EmbX.g4 by ANTLR 4.13.2


#include "EmbXVisitor.h"

#include "EmbXParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct EmbXParserStaticData final {
  EmbXParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  EmbXParserStaticData(const EmbXParserStaticData&) = delete;
  EmbXParserStaticData(EmbXParserStaticData&&) = delete;
  EmbXParserStaticData& operator=(const EmbXParserStaticData&) = delete;
  EmbXParserStaticData& operator=(EmbXParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag embxParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<EmbXParserStaticData> embxParserStaticData = nullptr;

void embxParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (embxParserStaticData != nullptr) {
    return;
  }
#else
  assert(embxParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<EmbXParserStaticData>(
    std::vector<std::string>{
      "module", "namespaceDecl", "importDecl", "qualifiedName", "item", 
      "endianDirective", "constDecl", "computedDecl", "enumDecl", "enumItem", 
      "typeAlias", "structDecl", "requiresDecl", "member", "virtualDecl", 
      "aliasDecl", "fieldDecl", "fieldModifier", "transform", "bitsBlock", 
      "bitField", "conditionalDecl", "variantDecl", "variantCase", "variantDefault", 
      "variantBody", "blockDecl", "atDecl", "alignDecl", "callbackDecl", 
      "parameterDecl", "callbackUse", "argList", "docComment", "attributeDecl", 
      "attributeType", "attributes", "attributeEntry", "typeRef", "terminatedSequenceSuffix", 
      "typeSuffix", "baseType", "primitive", "expr", "primary", "literal", 
      "endian"
    },
    std::vector<std::string>{
      "", "';'", "'='", "':'", "'{'", "','", "'}'", "'['", "']'", "'('", 
      "')'", "'-'", "'/'", "'%'", "'+'", "'=='", "'!='", "'<'", "'<='", 
      "'>'", "'>='", "'&&'", "'||'", "'*'", "'until'", "'max'", "'::'", 
      "'@'", "'attribute'", "'requires'", "'endian'", "'const'", "'computed'", 
      "'enum'", "'struct'", "'type'", "'bits'", "'variant'", "'by'", "'default'", 
      "'block'", "'at'", "'align'", "'callback'", "'param'", "'if'", "'else'", 
      "'let'", "'namespace'", "'import'", "'as'", "'alias'", "'transform'", 
      "'$next'", "'true'", "'false'", "'$size_in_bytes'", "'$min_size_in_bytes'", 
      "'$max_size_in_bytes'", "'little'", "'big'", "'native'", "'u8'", "'i8'", 
      "'u16'", "'i16'", "'u32'", "'i32'", "'u64'", "'i64'", "'f32'", "'f64'", 
      "'bytes'", "'string'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "STAR", "UNTIL", "MAX", "SCOPE", "AT", "ATTRIBUTE", 
      "REQUIRES", "ENDIAN", "CONST", "COMPUTED", "ENUM", "STRUCT", "TYPE", 
      "BITS", "VARIANT", "BY", "DEFAULT", "BLOCK", "ATCALL", "ALIGN", "CALLBACK", 
      "PARAM", "IF", "ELSE", "LET", "NAMESPACE", "IMPORT", "AS", "ALIAS", 
      "TRANSFORM", "NEXT", "TRUE", "FALSE", "SIZE_IN_BYTES", "MIN_SIZE_IN_BYTES", 
      "MAX_SIZE_IN_BYTES", "LITTLE", "BIG", "NATIVE", "U8", "I8", "U16", 
      "I16", "U32", "I32", "U64", "I64", "F32", "F64", "BYTES", "STRINGTYPE", 
      "ID", "HEX", "FLOAT", "INT", "STRING", "WS", "DOC_COMMENT", "LINECOMMENT", 
      "BLOCKCOMMENT"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,82,615,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,7,
  	42,2,43,7,43,2,44,7,44,2,45,7,45,2,46,7,46,1,0,3,0,96,8,0,1,0,3,0,99,
  	8,0,1,0,5,0,102,8,0,10,0,12,0,105,9,0,1,0,5,0,108,8,0,10,0,12,0,111,9,
  	0,1,0,1,0,1,1,1,1,1,1,1,1,1,2,1,2,1,2,1,2,3,2,123,8,2,1,2,1,2,1,3,1,3,
  	1,3,5,3,130,8,3,10,3,12,3,133,9,3,1,4,3,4,136,8,4,1,4,1,4,1,4,1,4,1,4,
  	1,4,1,4,1,4,1,4,3,4,147,8,4,1,5,1,5,1,5,1,5,1,6,3,6,154,8,6,1,6,1,6,1,
  	6,1,6,1,6,1,6,1,7,3,7,163,8,7,1,7,1,7,1,7,1,7,1,7,1,7,1,8,3,8,172,8,8,
  	1,8,1,8,1,8,1,8,3,8,178,8,8,1,8,1,8,1,8,1,8,5,8,184,8,8,10,8,12,8,187,
  	9,8,1,8,3,8,190,8,8,1,8,1,8,1,9,3,9,195,8,9,1,9,1,9,1,9,1,9,1,10,3,10,
  	202,8,10,1,10,1,10,1,10,1,10,1,10,1,10,1,11,3,11,211,8,11,1,11,1,11,1,
  	11,3,11,216,8,11,1,11,1,11,1,11,5,11,221,8,11,10,11,12,11,224,9,11,1,
  	11,1,11,1,12,1,12,1,12,1,12,1,13,3,13,233,8,13,1,13,1,13,1,13,1,13,1,
  	13,1,13,1,13,1,13,1,13,1,13,3,13,245,8,13,1,14,3,14,248,8,14,1,14,1,14,
  	1,14,1,14,1,14,1,14,1,15,3,15,257,8,15,1,15,1,15,1,15,1,15,1,15,1,15,
  	1,16,3,16,266,8,16,1,16,1,16,1,16,1,16,5,16,272,8,16,10,16,12,16,275,
  	9,16,1,16,3,16,278,8,16,1,16,1,16,3,16,282,8,16,1,16,1,16,1,17,1,17,1,
  	17,1,17,1,17,1,17,1,17,1,17,3,17,294,8,17,1,18,1,18,1,18,1,18,1,18,1,
  	18,5,18,302,8,18,10,18,12,18,305,9,18,1,18,1,18,1,19,3,19,310,8,19,1,
  	19,1,19,1,19,5,19,315,8,19,10,19,12,19,318,9,19,1,19,1,19,1,20,3,20,323,
  	8,20,1,20,1,20,1,20,1,20,1,20,1,20,1,20,5,20,332,8,20,10,20,12,20,335,
  	9,20,1,20,1,20,3,20,339,8,20,1,20,1,20,1,21,3,21,344,8,21,1,21,1,21,1,
  	21,1,21,1,21,1,21,5,21,352,8,21,10,21,12,21,355,9,21,1,21,1,21,1,21,1,
  	21,5,21,361,8,21,10,21,12,21,364,9,21,1,21,3,21,367,8,21,1,22,3,22,370,
  	8,22,1,22,1,22,1,22,1,22,1,22,1,22,4,22,378,8,22,11,22,12,22,379,1,22,
  	3,22,383,8,22,1,22,1,22,1,23,1,23,1,23,1,23,1,24,1,24,1,24,1,24,1,25,
  	1,25,1,25,1,25,1,25,5,25,400,8,25,10,25,12,25,403,9,25,1,25,3,25,406,
  	8,25,1,26,3,26,409,8,26,1,26,1,26,3,26,413,8,26,1,26,1,26,1,26,1,26,1,
  	26,5,26,420,8,26,10,26,12,26,423,9,26,1,26,1,26,1,27,3,27,428,8,27,1,
  	27,1,27,1,27,1,27,1,27,1,27,5,27,436,8,27,10,27,12,27,439,9,27,1,27,1,
  	27,1,28,3,28,444,8,28,1,28,1,28,1,28,1,28,1,28,1,28,1,29,3,29,453,8,29,
  	1,29,1,29,1,29,1,29,3,29,459,8,29,1,29,3,29,462,8,29,1,29,1,29,1,30,3,
  	30,467,8,30,1,30,1,30,1,30,1,30,1,30,1,30,1,31,3,31,476,8,31,1,31,1,31,
  	1,31,1,31,3,31,482,8,31,1,31,1,31,1,31,1,32,1,32,1,32,5,32,490,8,32,10,
  	32,12,32,493,9,32,1,33,4,33,496,8,33,11,33,12,33,497,1,34,1,34,1,34,1,
  	34,3,34,504,8,34,1,34,1,34,1,35,1,35,1,36,1,36,1,36,1,36,5,36,514,8,36,
  	10,36,12,36,517,9,36,1,36,3,36,520,8,36,1,36,1,36,1,37,1,37,1,37,1,37,
  	1,37,1,37,3,37,530,8,37,3,37,532,8,37,1,38,1,38,3,38,536,8,38,1,38,3,
  	38,539,8,38,1,38,3,38,542,8,38,1,39,1,39,4,39,546,8,39,11,39,12,39,547,
  	1,39,1,39,1,39,1,40,1,40,1,40,3,40,556,8,40,1,40,1,40,1,41,1,41,1,41,
  	1,41,3,41,564,8,41,1,42,1,42,1,43,1,43,1,43,1,43,3,43,572,8,43,1,43,1,
  	43,1,43,1,43,1,43,1,43,1,43,1,43,1,43,1,43,1,43,1,43,5,43,586,8,43,10,
  	43,12,43,589,9,43,1,44,1,44,1,44,1,44,1,44,1,44,1,44,1,44,1,44,1,44,1,
  	44,1,44,1,44,1,44,1,44,3,44,606,8,44,1,45,3,45,609,8,45,1,45,1,45,1,46,
  	1,46,1,46,0,1,86,47,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,
  	36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,
  	82,84,86,88,90,92,0,8,2,0,62,71,73,73,1,0,62,71,2,0,12,13,23,23,2,0,11,
  	11,14,14,1,0,15,20,1,0,21,22,1,0,75,78,1,0,59,61,673,0,95,1,0,0,0,2,114,
  	1,0,0,0,4,118,1,0,0,0,6,126,1,0,0,0,8,135,1,0,0,0,10,148,1,0,0,0,12,153,
  	1,0,0,0,14,162,1,0,0,0,16,171,1,0,0,0,18,194,1,0,0,0,20,201,1,0,0,0,22,
  	210,1,0,0,0,24,227,1,0,0,0,26,232,1,0,0,0,28,247,1,0,0,0,30,256,1,0,0,
  	0,32,265,1,0,0,0,34,293,1,0,0,0,36,295,1,0,0,0,38,309,1,0,0,0,40,322,
  	1,0,0,0,42,343,1,0,0,0,44,369,1,0,0,0,46,386,1,0,0,0,48,390,1,0,0,0,50,
  	405,1,0,0,0,52,408,1,0,0,0,54,427,1,0,0,0,56,443,1,0,0,0,58,452,1,0,0,
  	0,60,466,1,0,0,0,62,475,1,0,0,0,64,486,1,0,0,0,66,495,1,0,0,0,68,499,
  	1,0,0,0,70,507,1,0,0,0,72,509,1,0,0,0,74,523,1,0,0,0,76,535,1,0,0,0,78,
  	543,1,0,0,0,80,552,1,0,0,0,82,563,1,0,0,0,84,565,1,0,0,0,86,571,1,0,0,
  	0,88,605,1,0,0,0,90,608,1,0,0,0,92,612,1,0,0,0,94,96,5,80,0,0,95,94,1,
  	0,0,0,95,96,1,0,0,0,96,98,1,0,0,0,97,99,3,2,1,0,98,97,1,0,0,0,98,99,1,
  	0,0,0,99,103,1,0,0,0,100,102,3,4,2,0,101,100,1,0,0,0,102,105,1,0,0,0,
  	103,101,1,0,0,0,103,104,1,0,0,0,104,109,1,0,0,0,105,103,1,0,0,0,106,108,
  	3,8,4,0,107,106,1,0,0,0,108,111,1,0,0,0,109,107,1,0,0,0,109,110,1,0,0,
  	0,110,112,1,0,0,0,111,109,1,0,0,0,112,113,5,0,0,1,113,1,1,0,0,0,114,115,
  	5,48,0,0,115,116,3,6,3,0,116,117,5,1,0,0,117,3,1,0,0,0,118,119,5,49,0,
  	0,119,122,5,78,0,0,120,121,5,50,0,0,121,123,5,74,0,0,122,120,1,0,0,0,
  	122,123,1,0,0,0,123,124,1,0,0,0,124,125,5,1,0,0,125,5,1,0,0,0,126,131,
  	5,74,0,0,127,128,5,26,0,0,128,130,5,74,0,0,129,127,1,0,0,0,130,133,1,
  	0,0,0,131,129,1,0,0,0,131,132,1,0,0,0,132,7,1,0,0,0,133,131,1,0,0,0,134,
  	136,3,66,33,0,135,134,1,0,0,0,135,136,1,0,0,0,136,146,1,0,0,0,137,147,
  	3,68,34,0,138,147,3,10,5,0,139,147,3,12,6,0,140,147,3,14,7,0,141,147,
  	3,16,8,0,142,147,3,22,11,0,143,147,3,58,29,0,144,147,3,60,30,0,145,147,
  	3,20,10,0,146,137,1,0,0,0,146,138,1,0,0,0,146,139,1,0,0,0,146,140,1,0,
  	0,0,146,141,1,0,0,0,146,142,1,0,0,0,146,143,1,0,0,0,146,144,1,0,0,0,146,
  	145,1,0,0,0,147,9,1,0,0,0,148,149,5,27,0,0,149,150,5,30,0,0,150,151,3,
  	92,46,0,151,11,1,0,0,0,152,154,3,72,36,0,153,152,1,0,0,0,153,154,1,0,
  	0,0,154,155,1,0,0,0,155,156,5,31,0,0,156,157,5,74,0,0,157,158,5,2,0,0,
  	158,159,3,86,43,0,159,160,5,1,0,0,160,13,1,0,0,0,161,163,3,72,36,0,162,
  	161,1,0,0,0,162,163,1,0,0,0,163,164,1,0,0,0,164,165,5,32,0,0,165,166,
  	5,74,0,0,166,167,5,2,0,0,167,168,3,86,43,0,168,169,5,1,0,0,169,15,1,0,
  	0,0,170,172,3,72,36,0,171,170,1,0,0,0,171,172,1,0,0,0,172,173,1,0,0,0,
  	173,174,5,33,0,0,174,177,5,74,0,0,175,176,5,3,0,0,176,178,3,76,38,0,177,
  	175,1,0,0,0,177,178,1,0,0,0,178,179,1,0,0,0,179,180,5,4,0,0,180,185,3,
  	18,9,0,181,182,5,5,0,0,182,184,3,18,9,0,183,181,1,0,0,0,184,187,1,0,0,
  	0,185,183,1,0,0,0,185,186,1,0,0,0,186,189,1,0,0,0,187,185,1,0,0,0,188,
  	190,5,5,0,0,189,188,1,0,0,0,189,190,1,0,0,0,190,191,1,0,0,0,191,192,5,
  	6,0,0,192,17,1,0,0,0,193,195,3,66,33,0,194,193,1,0,0,0,194,195,1,0,0,
  	0,195,196,1,0,0,0,196,197,5,74,0,0,197,198,5,2,0,0,198,199,3,86,43,0,
  	199,19,1,0,0,0,200,202,3,72,36,0,201,200,1,0,0,0,201,202,1,0,0,0,202,
  	203,1,0,0,0,203,204,5,35,0,0,204,205,5,74,0,0,205,206,5,2,0,0,206,207,
  	3,76,38,0,207,208,5,1,0,0,208,21,1,0,0,0,209,211,3,72,36,0,210,209,1,
  	0,0,0,210,211,1,0,0,0,211,212,1,0,0,0,212,213,5,34,0,0,213,215,5,74,0,
  	0,214,216,3,92,46,0,215,214,1,0,0,0,215,216,1,0,0,0,216,217,1,0,0,0,217,
  	222,5,4,0,0,218,221,3,24,12,0,219,221,3,26,13,0,220,218,1,0,0,0,220,219,
  	1,0,0,0,221,224,1,0,0,0,222,220,1,0,0,0,222,223,1,0,0,0,223,225,1,0,0,
  	0,224,222,1,0,0,0,225,226,5,6,0,0,226,23,1,0,0,0,227,228,5,29,0,0,228,
  	229,3,86,43,0,229,230,5,1,0,0,230,25,1,0,0,0,231,233,3,66,33,0,232,231,
  	1,0,0,0,232,233,1,0,0,0,233,244,1,0,0,0,234,245,3,32,16,0,235,245,3,28,
  	14,0,236,245,3,30,15,0,237,245,3,38,19,0,238,245,3,44,22,0,239,245,3,
  	42,21,0,240,245,3,52,26,0,241,245,3,54,27,0,242,245,3,56,28,0,243,245,
  	3,62,31,0,244,234,1,0,0,0,244,235,1,0,0,0,244,236,1,0,0,0,244,237,1,0,
  	0,0,244,238,1,0,0,0,244,239,1,0,0,0,244,240,1,0,0,0,244,241,1,0,0,0,244,
  	242,1,0,0,0,244,243,1,0,0,0,245,27,1,0,0,0,246,248,3,72,36,0,247,246,
  	1,0,0,0,247,248,1,0,0,0,248,249,1,0,0,0,249,250,5,47,0,0,250,251,5,74,
  	0,0,251,252,5,2,0,0,252,253,3,86,43,0,253,254,5,1,0,0,254,29,1,0,0,0,
  	255,257,3,72,36,0,256,255,1,0,0,0,256,257,1,0,0,0,257,258,1,0,0,0,258,
  	259,5,51,0,0,259,260,5,74,0,0,260,261,5,2,0,0,261,262,5,74,0,0,262,263,
  	5,1,0,0,263,31,1,0,0,0,264,266,3,72,36,0,265,264,1,0,0,0,265,266,1,0,
  	0,0,266,267,1,0,0,0,267,268,5,74,0,0,268,269,5,3,0,0,269,273,3,76,38,
  	0,270,272,3,34,17,0,271,270,1,0,0,0,272,275,1,0,0,0,273,271,1,0,0,0,273,
  	274,1,0,0,0,274,277,1,0,0,0,275,273,1,0,0,0,276,278,3,36,18,0,277,276,
  	1,0,0,0,277,278,1,0,0,0,278,281,1,0,0,0,279,280,5,2,0,0,280,282,3,90,
  	45,0,281,279,1,0,0,0,281,282,1,0,0,0,282,283,1,0,0,0,283,284,5,1,0,0,
  	284,33,1,0,0,0,285,294,3,92,46,0,286,287,5,7,0,0,287,288,3,86,43,0,288,
  	289,5,8,0,0,289,294,1,0,0,0,290,291,5,9,0,0,291,292,5,77,0,0,292,294,
  	5,10,0,0,293,285,1,0,0,0,293,286,1,0,0,0,293,290,1,0,0,0,294,35,1,0,0,
  	0,295,296,5,52,0,0,296,297,5,74,0,0,297,298,5,9,0,0,298,303,3,86,43,0,
  	299,300,5,5,0,0,300,302,3,86,43,0,301,299,1,0,0,0,302,305,1,0,0,0,303,
  	301,1,0,0,0,303,304,1,0,0,0,304,306,1,0,0,0,305,303,1,0,0,0,306,307,5,
  	10,0,0,307,37,1,0,0,0,308,310,3,72,36,0,309,308,1,0,0,0,309,310,1,0,0,
  	0,310,311,1,0,0,0,311,312,5,36,0,0,312,316,5,4,0,0,313,315,3,40,20,0,
  	314,313,1,0,0,0,315,318,1,0,0,0,316,314,1,0,0,0,316,317,1,0,0,0,317,319,
  	1,0,0,0,318,316,1,0,0,0,319,320,5,6,0,0,320,39,1,0,0,0,321,323,3,66,33,
  	0,322,321,1,0,0,0,322,323,1,0,0,0,323,324,1,0,0,0,324,325,5,74,0,0,325,
  	326,5,3,0,0,326,327,3,76,38,0,327,328,5,9,0,0,328,329,5,77,0,0,329,333,
  	5,10,0,0,330,332,3,34,17,0,331,330,1,0,0,0,332,335,1,0,0,0,333,331,1,
  	0,0,0,333,334,1,0,0,0,334,338,1,0,0,0,335,333,1,0,0,0,336,337,5,2,0,0,
  	337,339,3,90,45,0,338,336,1,0,0,0,338,339,1,0,0,0,339,340,1,0,0,0,340,
  	341,5,1,0,0,341,41,1,0,0,0,342,344,3,72,36,0,343,342,1,0,0,0,343,344,
  	1,0,0,0,344,345,1,0,0,0,345,346,5,45,0,0,346,347,5,9,0,0,347,348,3,86,
  	43,0,348,349,5,10,0,0,349,353,5,4,0,0,350,352,3,26,13,0,351,350,1,0,0,
  	0,352,355,1,0,0,0,353,351,1,0,0,0,353,354,1,0,0,0,354,356,1,0,0,0,355,
  	353,1,0,0,0,356,366,5,6,0,0,357,358,5,46,0,0,358,362,5,4,0,0,359,361,
  	3,26,13,0,360,359,1,0,0,0,361,364,1,0,0,0,362,360,1,0,0,0,362,363,1,0,
  	0,0,363,365,1,0,0,0,364,362,1,0,0,0,365,367,5,6,0,0,366,357,1,0,0,0,366,
  	367,1,0,0,0,367,43,1,0,0,0,368,370,3,72,36,0,369,368,1,0,0,0,369,370,
  	1,0,0,0,370,371,1,0,0,0,371,372,5,37,0,0,372,373,5,74,0,0,373,374,5,38,
  	0,0,374,375,3,86,43,0,375,377,5,4,0,0,376,378,3,46,23,0,377,376,1,0,0,
  	0,378,379,1,0,0,0,379,377,1,0,0,0,379,380,1,0,0,0,380,382,1,0,0,0,381,
  	383,3,48,24,0,382,381,1,0,0,0,382,383,1,0,0,0,383,384,1,0,0,0,384,385,
  	5,6,0,0,385,45,1,0,0,0,386,387,3,86,43,0,387,388,5,3,0,0,388,389,3,50,
  	25,0,389,47,1,0,0,0,390,391,5,39,0,0,391,392,5,3,0,0,392,393,3,50,25,
  	0,393,49,1,0,0,0,394,395,3,76,38,0,395,396,5,1,0,0,396,406,1,0,0,0,397,
  	401,5,4,0,0,398,400,3,26,13,0,399,398,1,0,0,0,400,403,1,0,0,0,401,399,
  	1,0,0,0,401,402,1,0,0,0,402,404,1,0,0,0,403,401,1,0,0,0,404,406,5,6,0,
  	0,405,394,1,0,0,0,405,397,1,0,0,0,406,51,1,0,0,0,407,409,3,72,36,0,408,
  	407,1,0,0,0,408,409,1,0,0,0,409,410,1,0,0,0,410,412,5,40,0,0,411,413,
  	5,74,0,0,412,411,1,0,0,0,412,413,1,0,0,0,413,414,1,0,0,0,414,415,5,7,
  	0,0,415,416,3,86,43,0,416,417,5,8,0,0,417,421,5,4,0,0,418,420,3,26,13,
  	0,419,418,1,0,0,0,420,423,1,0,0,0,421,419,1,0,0,0,421,422,1,0,0,0,422,
  	424,1,0,0,0,423,421,1,0,0,0,424,425,5,6,0,0,425,53,1,0,0,0,426,428,3,
  	72,36,0,427,426,1,0,0,0,427,428,1,0,0,0,428,429,1,0,0,0,429,430,5,41,
  	0,0,430,431,5,9,0,0,431,432,3,86,43,0,432,433,5,10,0,0,433,437,5,4,0,
  	0,434,436,3,26,13,0,435,434,1,0,0,0,436,439,1,0,0,0,437,435,1,0,0,0,437,
  	438,1,0,0,0,438,440,1,0,0,0,439,437,1,0,0,0,440,441,5,6,0,0,441,55,1,
  	0,0,0,442,444,3,72,36,0,443,442,1,0,0,0,443,444,1,0,0,0,444,445,1,0,0,
  	0,445,446,5,42,0,0,446,447,5,9,0,0,447,448,3,86,43,0,448,449,5,10,0,0,
  	449,450,5,1,0,0,450,57,1,0,0,0,451,453,3,72,36,0,452,451,1,0,0,0,452,
  	453,1,0,0,0,453,454,1,0,0,0,454,455,5,43,0,0,455,461,5,74,0,0,456,458,
  	5,9,0,0,457,459,5,74,0,0,458,457,1,0,0,0,458,459,1,0,0,0,459,460,1,0,
  	0,0,460,462,5,10,0,0,461,456,1,0,0,0,461,462,1,0,0,0,462,463,1,0,0,0,
  	463,464,5,1,0,0,464,59,1,0,0,0,465,467,3,72,36,0,466,465,1,0,0,0,466,
  	467,1,0,0,0,467,468,1,0,0,0,468,469,5,44,0,0,469,470,5,74,0,0,470,471,
  	5,3,0,0,471,472,3,76,38,0,472,473,5,1,0,0,473,61,1,0,0,0,474,476,3,72,
  	36,0,475,474,1,0,0,0,475,476,1,0,0,0,476,477,1,0,0,0,477,478,5,43,0,0,
  	478,479,5,74,0,0,479,481,5,9,0,0,480,482,3,64,32,0,481,480,1,0,0,0,481,
  	482,1,0,0,0,482,483,1,0,0,0,483,484,5,10,0,0,484,485,5,1,0,0,485,63,1,
  	0,0,0,486,491,3,86,43,0,487,488,5,5,0,0,488,490,3,86,43,0,489,487,1,0,
  	0,0,490,493,1,0,0,0,491,489,1,0,0,0,491,492,1,0,0,0,492,65,1,0,0,0,493,
  	491,1,0,0,0,494,496,5,80,0,0,495,494,1,0,0,0,496,497,1,0,0,0,497,495,
  	1,0,0,0,497,498,1,0,0,0,498,67,1,0,0,0,499,500,5,28,0,0,500,503,5,74,
  	0,0,501,502,5,3,0,0,502,504,3,70,35,0,503,501,1,0,0,0,503,504,1,0,0,0,
  	504,505,1,0,0,0,505,506,5,1,0,0,506,69,1,0,0,0,507,508,7,0,0,0,508,71,
  	1,0,0,0,509,510,5,7,0,0,510,515,3,74,37,0,511,512,5,5,0,0,512,514,3,74,
  	37,0,513,511,1,0,0,0,514,517,1,0,0,0,515,513,1,0,0,0,515,516,1,0,0,0,
  	516,519,1,0,0,0,517,515,1,0,0,0,518,520,5,5,0,0,519,518,1,0,0,0,519,520,
  	1,0,0,0,520,521,1,0,0,0,521,522,5,8,0,0,522,73,1,0,0,0,523,531,5,74,0,
  	0,524,529,5,2,0,0,525,530,3,90,45,0,526,530,5,74,0,0,527,530,5,54,0,0,
  	528,530,5,55,0,0,529,525,1,0,0,0,529,526,1,0,0,0,529,527,1,0,0,0,529,
  	528,1,0,0,0,530,532,1,0,0,0,531,524,1,0,0,0,531,532,1,0,0,0,532,75,1,
  	0,0,0,533,536,3,6,3,0,534,536,3,82,41,0,535,533,1,0,0,0,535,534,1,0,0,
  	0,536,538,1,0,0,0,537,539,3,80,40,0,538,537,1,0,0,0,538,539,1,0,0,0,539,
  	541,1,0,0,0,540,542,3,78,39,0,541,540,1,0,0,0,541,542,1,0,0,0,542,77,
  	1,0,0,0,543,545,5,24,0,0,544,546,5,75,0,0,545,544,1,0,0,0,546,547,1,0,
  	0,0,547,545,1,0,0,0,547,548,1,0,0,0,548,549,1,0,0,0,549,550,5,25,0,0,
  	550,551,5,77,0,0,551,79,1,0,0,0,552,555,5,7,0,0,553,556,5,23,0,0,554,
  	556,3,86,43,0,555,553,1,0,0,0,555,554,1,0,0,0,556,557,1,0,0,0,557,558,
  	5,8,0,0,558,81,1,0,0,0,559,564,3,84,42,0,560,564,5,74,0,0,561,564,5,72,
  	0,0,562,564,5,73,0,0,563,559,1,0,0,0,563,560,1,0,0,0,563,561,1,0,0,0,
  	563,562,1,0,0,0,564,83,1,0,0,0,565,566,7,1,0,0,566,85,1,0,0,0,567,568,
  	6,43,-1,0,568,572,3,88,44,0,569,570,5,11,0,0,570,572,3,86,43,5,571,567,
  	1,0,0,0,571,569,1,0,0,0,572,587,1,0,0,0,573,574,10,4,0,0,574,575,7,2,
  	0,0,575,586,3,86,43,5,576,577,10,3,0,0,577,578,7,3,0,0,578,586,3,86,43,
  	4,579,580,10,2,0,0,580,581,7,4,0,0,581,586,3,86,43,3,582,583,10,1,0,0,
  	583,584,7,5,0,0,584,586,3,86,43,2,585,573,1,0,0,0,585,576,1,0,0,0,585,
  	579,1,0,0,0,585,582,1,0,0,0,586,589,1,0,0,0,587,585,1,0,0,0,587,588,1,
  	0,0,0,588,87,1,0,0,0,589,587,1,0,0,0,590,606,5,77,0,0,591,606,5,75,0,
  	0,592,606,5,76,0,0,593,606,5,78,0,0,594,606,5,54,0,0,595,606,5,55,0,0,
  	596,606,5,53,0,0,597,606,5,56,0,0,598,606,5,57,0,0,599,606,5,58,0,0,600,
  	606,3,6,3,0,601,602,5,9,0,0,602,603,3,86,43,0,603,604,5,10,0,0,604,606,
  	1,0,0,0,605,590,1,0,0,0,605,591,1,0,0,0,605,592,1,0,0,0,605,593,1,0,0,
  	0,605,594,1,0,0,0,605,595,1,0,0,0,605,596,1,0,0,0,605,597,1,0,0,0,605,
  	598,1,0,0,0,605,599,1,0,0,0,605,600,1,0,0,0,605,601,1,0,0,0,606,89,1,
  	0,0,0,607,609,5,11,0,0,608,607,1,0,0,0,608,609,1,0,0,0,609,610,1,0,0,
  	0,610,611,7,6,0,0,611,91,1,0,0,0,612,613,7,7,0,0,613,93,1,0,0,0,74,95,
  	98,103,109,122,131,135,146,153,162,171,177,185,189,194,201,210,215,220,
  	222,232,244,247,256,265,273,277,281,293,303,309,316,322,333,338,343,353,
  	362,366,369,379,382,401,405,408,412,421,427,437,443,452,458,461,466,475,
  	481,491,497,503,515,519,529,531,535,538,541,547,555,563,571,585,587,605,
  	608
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  embxParserStaticData = std::move(staticData);
}

}

EmbXParser::EmbXParser(TokenStream *input) : EmbXParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

EmbXParser::EmbXParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  EmbXParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *embxParserStaticData->atn, embxParserStaticData->decisionToDFA, embxParserStaticData->sharedContextCache, options);
}

EmbXParser::~EmbXParser() {
  delete _interpreter;
}

const atn::ATN& EmbXParser::getATN() const {
  return *embxParserStaticData->atn;
}

std::string EmbXParser::getGrammarFileName() const {
  return "EmbX.g4";
}

const std::vector<std::string>& EmbXParser::getRuleNames() const {
  return embxParserStaticData->ruleNames;
}

const dfa::Vocabulary& EmbXParser::getVocabulary() const {
  return embxParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView EmbXParser::getSerializedATN() const {
  return embxParserStaticData->serializedATN;
}


//----------------- ModuleContext ------------------------------------------------------------------

EmbXParser::ModuleContext::ModuleContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::ModuleContext::EOF() {
  return getToken(EmbXParser::EOF, 0);
}

tree::TerminalNode* EmbXParser::ModuleContext::DOC_COMMENT() {
  return getToken(EmbXParser::DOC_COMMENT, 0);
}

EmbXParser::NamespaceDeclContext* EmbXParser::ModuleContext::namespaceDecl() {
  return getRuleContext<EmbXParser::NamespaceDeclContext>(0);
}

std::vector<EmbXParser::ImportDeclContext *> EmbXParser::ModuleContext::importDecl() {
  return getRuleContexts<EmbXParser::ImportDeclContext>();
}

EmbXParser::ImportDeclContext* EmbXParser::ModuleContext::importDecl(size_t i) {
  return getRuleContext<EmbXParser::ImportDeclContext>(i);
}

std::vector<EmbXParser::ItemContext *> EmbXParser::ModuleContext::item() {
  return getRuleContexts<EmbXParser::ItemContext>();
}

EmbXParser::ItemContext* EmbXParser::ModuleContext::item(size_t i) {
  return getRuleContext<EmbXParser::ItemContext>(i);
}


size_t EmbXParser::ModuleContext::getRuleIndex() const {
  return EmbXParser::RuleModule;
}


std::any EmbXParser::ModuleContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitModule(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::ModuleContext* EmbXParser::module() {
  ModuleContext *_localctx = _tracker.createInstance<ModuleContext>(_ctx, getState());
  enterRule(_localctx, 0, EmbXParser::RuleModule);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(95);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx)) {
    case 1: {
      setState(94);
      match(EmbXParser::DOC_COMMENT);
      break;
    }

    default:
      break;
    }
    setState(98);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::NAMESPACE) {
      setState(97);
      namespaceDecl();
    }
    setState(103);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EmbXParser::IMPORT) {
      setState(100);
      importDecl();
      setState(105);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(109);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 26455253713024) != 0) || _la == EmbXParser::DOC_COMMENT) {
      setState(106);
      item();
      setState(111);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(112);
    match(EmbXParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NamespaceDeclContext ------------------------------------------------------------------

EmbXParser::NamespaceDeclContext::NamespaceDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::NamespaceDeclContext::NAMESPACE() {
  return getToken(EmbXParser::NAMESPACE, 0);
}

EmbXParser::QualifiedNameContext* EmbXParser::NamespaceDeclContext::qualifiedName() {
  return getRuleContext<EmbXParser::QualifiedNameContext>(0);
}


size_t EmbXParser::NamespaceDeclContext::getRuleIndex() const {
  return EmbXParser::RuleNamespaceDecl;
}


std::any EmbXParser::NamespaceDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitNamespaceDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::NamespaceDeclContext* EmbXParser::namespaceDecl() {
  NamespaceDeclContext *_localctx = _tracker.createInstance<NamespaceDeclContext>(_ctx, getState());
  enterRule(_localctx, 2, EmbXParser::RuleNamespaceDecl);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(114);
    match(EmbXParser::NAMESPACE);
    setState(115);
    qualifiedName();
    setState(116);
    match(EmbXParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ImportDeclContext ------------------------------------------------------------------

EmbXParser::ImportDeclContext::ImportDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::ImportDeclContext::IMPORT() {
  return getToken(EmbXParser::IMPORT, 0);
}

tree::TerminalNode* EmbXParser::ImportDeclContext::STRING() {
  return getToken(EmbXParser::STRING, 0);
}

tree::TerminalNode* EmbXParser::ImportDeclContext::AS() {
  return getToken(EmbXParser::AS, 0);
}

tree::TerminalNode* EmbXParser::ImportDeclContext::ID() {
  return getToken(EmbXParser::ID, 0);
}


size_t EmbXParser::ImportDeclContext::getRuleIndex() const {
  return EmbXParser::RuleImportDecl;
}


std::any EmbXParser::ImportDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitImportDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::ImportDeclContext* EmbXParser::importDecl() {
  ImportDeclContext *_localctx = _tracker.createInstance<ImportDeclContext>(_ctx, getState());
  enterRule(_localctx, 4, EmbXParser::RuleImportDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(118);
    match(EmbXParser::IMPORT);
    setState(119);
    match(EmbXParser::STRING);
    setState(122);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::AS) {
      setState(120);
      match(EmbXParser::AS);
      setState(121);
      match(EmbXParser::ID);
    }
    setState(124);
    match(EmbXParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- QualifiedNameContext ------------------------------------------------------------------

EmbXParser::QualifiedNameContext::QualifiedNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> EmbXParser::QualifiedNameContext::ID() {
  return getTokens(EmbXParser::ID);
}

tree::TerminalNode* EmbXParser::QualifiedNameContext::ID(size_t i) {
  return getToken(EmbXParser::ID, i);
}

std::vector<tree::TerminalNode *> EmbXParser::QualifiedNameContext::SCOPE() {
  return getTokens(EmbXParser::SCOPE);
}

tree::TerminalNode* EmbXParser::QualifiedNameContext::SCOPE(size_t i) {
  return getToken(EmbXParser::SCOPE, i);
}


size_t EmbXParser::QualifiedNameContext::getRuleIndex() const {
  return EmbXParser::RuleQualifiedName;
}


std::any EmbXParser::QualifiedNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitQualifiedName(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::QualifiedNameContext* EmbXParser::qualifiedName() {
  QualifiedNameContext *_localctx = _tracker.createInstance<QualifiedNameContext>(_ctx, getState());
  enterRule(_localctx, 6, EmbXParser::RuleQualifiedName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(126);
    match(EmbXParser::ID);
    setState(131);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(127);
        match(EmbXParser::SCOPE);
        setState(128);
        match(EmbXParser::ID); 
      }
      setState(133);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ItemContext ------------------------------------------------------------------

EmbXParser::ItemContext::ItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EmbXParser::AttributeDeclContext* EmbXParser::ItemContext::attributeDecl() {
  return getRuleContext<EmbXParser::AttributeDeclContext>(0);
}

EmbXParser::EndianDirectiveContext* EmbXParser::ItemContext::endianDirective() {
  return getRuleContext<EmbXParser::EndianDirectiveContext>(0);
}

EmbXParser::ConstDeclContext* EmbXParser::ItemContext::constDecl() {
  return getRuleContext<EmbXParser::ConstDeclContext>(0);
}

EmbXParser::ComputedDeclContext* EmbXParser::ItemContext::computedDecl() {
  return getRuleContext<EmbXParser::ComputedDeclContext>(0);
}

EmbXParser::EnumDeclContext* EmbXParser::ItemContext::enumDecl() {
  return getRuleContext<EmbXParser::EnumDeclContext>(0);
}

EmbXParser::StructDeclContext* EmbXParser::ItemContext::structDecl() {
  return getRuleContext<EmbXParser::StructDeclContext>(0);
}

EmbXParser::CallbackDeclContext* EmbXParser::ItemContext::callbackDecl() {
  return getRuleContext<EmbXParser::CallbackDeclContext>(0);
}

EmbXParser::ParameterDeclContext* EmbXParser::ItemContext::parameterDecl() {
  return getRuleContext<EmbXParser::ParameterDeclContext>(0);
}

EmbXParser::TypeAliasContext* EmbXParser::ItemContext::typeAlias() {
  return getRuleContext<EmbXParser::TypeAliasContext>(0);
}

EmbXParser::DocCommentContext* EmbXParser::ItemContext::docComment() {
  return getRuleContext<EmbXParser::DocCommentContext>(0);
}


size_t EmbXParser::ItemContext::getRuleIndex() const {
  return EmbXParser::RuleItem;
}


std::any EmbXParser::ItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitItem(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::ItemContext* EmbXParser::item() {
  ItemContext *_localctx = _tracker.createInstance<ItemContext>(_ctx, getState());
  enterRule(_localctx, 8, EmbXParser::RuleItem);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(135);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::DOC_COMMENT) {
      setState(134);
      docComment();
    }
    setState(146);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 7, _ctx)) {
    case 1: {
      setState(137);
      attributeDecl();
      break;
    }

    case 2: {
      setState(138);
      endianDirective();
      break;
    }

    case 3: {
      setState(139);
      constDecl();
      break;
    }

    case 4: {
      setState(140);
      computedDecl();
      break;
    }

    case 5: {
      setState(141);
      enumDecl();
      break;
    }

    case 6: {
      setState(142);
      structDecl();
      break;
    }

    case 7: {
      setState(143);
      callbackDecl();
      break;
    }

    case 8: {
      setState(144);
      parameterDecl();
      break;
    }

    case 9: {
      setState(145);
      typeAlias();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EndianDirectiveContext ------------------------------------------------------------------

EmbXParser::EndianDirectiveContext::EndianDirectiveContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::EndianDirectiveContext::AT() {
  return getToken(EmbXParser::AT, 0);
}

tree::TerminalNode* EmbXParser::EndianDirectiveContext::ENDIAN() {
  return getToken(EmbXParser::ENDIAN, 0);
}

EmbXParser::EndianContext* EmbXParser::EndianDirectiveContext::endian() {
  return getRuleContext<EmbXParser::EndianContext>(0);
}


size_t EmbXParser::EndianDirectiveContext::getRuleIndex() const {
  return EmbXParser::RuleEndianDirective;
}


std::any EmbXParser::EndianDirectiveContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitEndianDirective(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::EndianDirectiveContext* EmbXParser::endianDirective() {
  EndianDirectiveContext *_localctx = _tracker.createInstance<EndianDirectiveContext>(_ctx, getState());
  enterRule(_localctx, 10, EmbXParser::RuleEndianDirective);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(148);
    match(EmbXParser::AT);
    setState(149);
    match(EmbXParser::ENDIAN);
    setState(150);
    endian();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstDeclContext ------------------------------------------------------------------

EmbXParser::ConstDeclContext::ConstDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::ConstDeclContext::CONST() {
  return getToken(EmbXParser::CONST, 0);
}

tree::TerminalNode* EmbXParser::ConstDeclContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

EmbXParser::ExprContext* EmbXParser::ConstDeclContext::expr() {
  return getRuleContext<EmbXParser::ExprContext>(0);
}

EmbXParser::AttributesContext* EmbXParser::ConstDeclContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}


size_t EmbXParser::ConstDeclContext::getRuleIndex() const {
  return EmbXParser::RuleConstDecl;
}


std::any EmbXParser::ConstDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitConstDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::ConstDeclContext* EmbXParser::constDecl() {
  ConstDeclContext *_localctx = _tracker.createInstance<ConstDeclContext>(_ctx, getState());
  enterRule(_localctx, 12, EmbXParser::RuleConstDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(153);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(152);
      attributes();
    }
    setState(155);
    match(EmbXParser::CONST);
    setState(156);
    match(EmbXParser::ID);
    setState(157);
    match(EmbXParser::T__1);
    setState(158);
    expr(0);
    setState(159);
    match(EmbXParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ComputedDeclContext ------------------------------------------------------------------

EmbXParser::ComputedDeclContext::ComputedDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::ComputedDeclContext::COMPUTED() {
  return getToken(EmbXParser::COMPUTED, 0);
}

tree::TerminalNode* EmbXParser::ComputedDeclContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

EmbXParser::ExprContext* EmbXParser::ComputedDeclContext::expr() {
  return getRuleContext<EmbXParser::ExprContext>(0);
}

EmbXParser::AttributesContext* EmbXParser::ComputedDeclContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}


size_t EmbXParser::ComputedDeclContext::getRuleIndex() const {
  return EmbXParser::RuleComputedDecl;
}


std::any EmbXParser::ComputedDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitComputedDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::ComputedDeclContext* EmbXParser::computedDecl() {
  ComputedDeclContext *_localctx = _tracker.createInstance<ComputedDeclContext>(_ctx, getState());
  enterRule(_localctx, 14, EmbXParser::RuleComputedDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(162);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(161);
      attributes();
    }
    setState(164);
    match(EmbXParser::COMPUTED);
    setState(165);
    match(EmbXParser::ID);
    setState(166);
    match(EmbXParser::T__1);
    setState(167);
    expr(0);
    setState(168);
    match(EmbXParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EnumDeclContext ------------------------------------------------------------------

EmbXParser::EnumDeclContext::EnumDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::EnumDeclContext::ENUM() {
  return getToken(EmbXParser::ENUM, 0);
}

tree::TerminalNode* EmbXParser::EnumDeclContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

std::vector<EmbXParser::EnumItemContext *> EmbXParser::EnumDeclContext::enumItem() {
  return getRuleContexts<EmbXParser::EnumItemContext>();
}

EmbXParser::EnumItemContext* EmbXParser::EnumDeclContext::enumItem(size_t i) {
  return getRuleContext<EmbXParser::EnumItemContext>(i);
}

EmbXParser::AttributesContext* EmbXParser::EnumDeclContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}

EmbXParser::TypeRefContext* EmbXParser::EnumDeclContext::typeRef() {
  return getRuleContext<EmbXParser::TypeRefContext>(0);
}


size_t EmbXParser::EnumDeclContext::getRuleIndex() const {
  return EmbXParser::RuleEnumDecl;
}


std::any EmbXParser::EnumDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitEnumDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::EnumDeclContext* EmbXParser::enumDecl() {
  EnumDeclContext *_localctx = _tracker.createInstance<EnumDeclContext>(_ctx, getState());
  enterRule(_localctx, 16, EmbXParser::RuleEnumDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(171);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(170);
      attributes();
    }
    setState(173);
    match(EmbXParser::ENUM);
    setState(174);
    match(EmbXParser::ID);
    setState(177);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__2) {
      setState(175);
      match(EmbXParser::T__2);
      setState(176);
      typeRef();
    }
    setState(179);
    match(EmbXParser::T__3);
    setState(180);
    enumItem();
    setState(185);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(181);
        match(EmbXParser::T__4);
        setState(182);
        enumItem(); 
      }
      setState(187);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx);
    }
    setState(189);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__4) {
      setState(188);
      match(EmbXParser::T__4);
    }
    setState(191);
    match(EmbXParser::T__5);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EnumItemContext ------------------------------------------------------------------

EmbXParser::EnumItemContext::EnumItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::EnumItemContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

EmbXParser::ExprContext* EmbXParser::EnumItemContext::expr() {
  return getRuleContext<EmbXParser::ExprContext>(0);
}

EmbXParser::DocCommentContext* EmbXParser::EnumItemContext::docComment() {
  return getRuleContext<EmbXParser::DocCommentContext>(0);
}


size_t EmbXParser::EnumItemContext::getRuleIndex() const {
  return EmbXParser::RuleEnumItem;
}


std::any EmbXParser::EnumItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitEnumItem(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::EnumItemContext* EmbXParser::enumItem() {
  EnumItemContext *_localctx = _tracker.createInstance<EnumItemContext>(_ctx, getState());
  enterRule(_localctx, 18, EmbXParser::RuleEnumItem);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(194);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::DOC_COMMENT) {
      setState(193);
      docComment();
    }
    setState(196);
    match(EmbXParser::ID);
    setState(197);
    match(EmbXParser::T__1);
    setState(198);
    expr(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeAliasContext ------------------------------------------------------------------

EmbXParser::TypeAliasContext::TypeAliasContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::TypeAliasContext::TYPE() {
  return getToken(EmbXParser::TYPE, 0);
}

tree::TerminalNode* EmbXParser::TypeAliasContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

EmbXParser::TypeRefContext* EmbXParser::TypeAliasContext::typeRef() {
  return getRuleContext<EmbXParser::TypeRefContext>(0);
}

EmbXParser::AttributesContext* EmbXParser::TypeAliasContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}


size_t EmbXParser::TypeAliasContext::getRuleIndex() const {
  return EmbXParser::RuleTypeAlias;
}


std::any EmbXParser::TypeAliasContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitTypeAlias(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::TypeAliasContext* EmbXParser::typeAlias() {
  TypeAliasContext *_localctx = _tracker.createInstance<TypeAliasContext>(_ctx, getState());
  enterRule(_localctx, 20, EmbXParser::RuleTypeAlias);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(201);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(200);
      attributes();
    }
    setState(203);
    match(EmbXParser::TYPE);
    setState(204);
    match(EmbXParser::ID);
    setState(205);
    match(EmbXParser::T__1);
    setState(206);
    typeRef();
    setState(207);
    match(EmbXParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StructDeclContext ------------------------------------------------------------------

EmbXParser::StructDeclContext::StructDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::StructDeclContext::STRUCT() {
  return getToken(EmbXParser::STRUCT, 0);
}

tree::TerminalNode* EmbXParser::StructDeclContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

EmbXParser::AttributesContext* EmbXParser::StructDeclContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}

EmbXParser::EndianContext* EmbXParser::StructDeclContext::endian() {
  return getRuleContext<EmbXParser::EndianContext>(0);
}

std::vector<EmbXParser::RequiresDeclContext *> EmbXParser::StructDeclContext::requiresDecl() {
  return getRuleContexts<EmbXParser::RequiresDeclContext>();
}

EmbXParser::RequiresDeclContext* EmbXParser::StructDeclContext::requiresDecl(size_t i) {
  return getRuleContext<EmbXParser::RequiresDeclContext>(i);
}

std::vector<EmbXParser::MemberContext *> EmbXParser::StructDeclContext::member() {
  return getRuleContexts<EmbXParser::MemberContext>();
}

EmbXParser::MemberContext* EmbXParser::StructDeclContext::member(size_t i) {
  return getRuleContext<EmbXParser::MemberContext>(i);
}


size_t EmbXParser::StructDeclContext::getRuleIndex() const {
  return EmbXParser::RuleStructDecl;
}


std::any EmbXParser::StructDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitStructDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::StructDeclContext* EmbXParser::structDecl() {
  StructDeclContext *_localctx = _tracker.createInstance<StructDeclContext>(_ctx, getState());
  enterRule(_localctx, 22, EmbXParser::RuleStructDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(210);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(209);
      attributes();
    }
    setState(212);
    match(EmbXParser::STRUCT);
    setState(213);
    match(EmbXParser::ID);
    setState(215);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 4035225266123964416) != 0)) {
      setState(214);
      endian();
    }
    setState(217);
    match(EmbXParser::T__3);
    setState(222);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 2444421043847296) != 0) || _la == EmbXParser::ID

    || _la == EmbXParser::DOC_COMMENT) {
      setState(220);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case EmbXParser::REQUIRES: {
          setState(218);
          requiresDecl();
          break;
        }

        case EmbXParser::T__6:
        case EmbXParser::BITS:
        case EmbXParser::VARIANT:
        case EmbXParser::BLOCK:
        case EmbXParser::ATCALL:
        case EmbXParser::ALIGN:
        case EmbXParser::CALLBACK:
        case EmbXParser::IF:
        case EmbXParser::LET:
        case EmbXParser::ALIAS:
        case EmbXParser::ID:
        case EmbXParser::DOC_COMMENT: {
          setState(219);
          member();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(224);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(225);
    match(EmbXParser::T__5);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RequiresDeclContext ------------------------------------------------------------------

EmbXParser::RequiresDeclContext::RequiresDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::RequiresDeclContext::REQUIRES() {
  return getToken(EmbXParser::REQUIRES, 0);
}

EmbXParser::ExprContext* EmbXParser::RequiresDeclContext::expr() {
  return getRuleContext<EmbXParser::ExprContext>(0);
}


size_t EmbXParser::RequiresDeclContext::getRuleIndex() const {
  return EmbXParser::RuleRequiresDecl;
}


std::any EmbXParser::RequiresDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitRequiresDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::RequiresDeclContext* EmbXParser::requiresDecl() {
  RequiresDeclContext *_localctx = _tracker.createInstance<RequiresDeclContext>(_ctx, getState());
  enterRule(_localctx, 24, EmbXParser::RuleRequiresDecl);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(227);
    match(EmbXParser::REQUIRES);
    setState(228);
    expr(0);
    setState(229);
    match(EmbXParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MemberContext ------------------------------------------------------------------

EmbXParser::MemberContext::MemberContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EmbXParser::FieldDeclContext* EmbXParser::MemberContext::fieldDecl() {
  return getRuleContext<EmbXParser::FieldDeclContext>(0);
}

EmbXParser::VirtualDeclContext* EmbXParser::MemberContext::virtualDecl() {
  return getRuleContext<EmbXParser::VirtualDeclContext>(0);
}

EmbXParser::AliasDeclContext* EmbXParser::MemberContext::aliasDecl() {
  return getRuleContext<EmbXParser::AliasDeclContext>(0);
}

EmbXParser::BitsBlockContext* EmbXParser::MemberContext::bitsBlock() {
  return getRuleContext<EmbXParser::BitsBlockContext>(0);
}

EmbXParser::VariantDeclContext* EmbXParser::MemberContext::variantDecl() {
  return getRuleContext<EmbXParser::VariantDeclContext>(0);
}

EmbXParser::ConditionalDeclContext* EmbXParser::MemberContext::conditionalDecl() {
  return getRuleContext<EmbXParser::ConditionalDeclContext>(0);
}

EmbXParser::BlockDeclContext* EmbXParser::MemberContext::blockDecl() {
  return getRuleContext<EmbXParser::BlockDeclContext>(0);
}

EmbXParser::AtDeclContext* EmbXParser::MemberContext::atDecl() {
  return getRuleContext<EmbXParser::AtDeclContext>(0);
}

EmbXParser::AlignDeclContext* EmbXParser::MemberContext::alignDecl() {
  return getRuleContext<EmbXParser::AlignDeclContext>(0);
}

EmbXParser::CallbackUseContext* EmbXParser::MemberContext::callbackUse() {
  return getRuleContext<EmbXParser::CallbackUseContext>(0);
}

EmbXParser::DocCommentContext* EmbXParser::MemberContext::docComment() {
  return getRuleContext<EmbXParser::DocCommentContext>(0);
}


size_t EmbXParser::MemberContext::getRuleIndex() const {
  return EmbXParser::RuleMember;
}


std::any EmbXParser::MemberContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitMember(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::MemberContext* EmbXParser::member() {
  MemberContext *_localctx = _tracker.createInstance<MemberContext>(_ctx, getState());
  enterRule(_localctx, 26, EmbXParser::RuleMember);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(232);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::DOC_COMMENT) {
      setState(231);
      docComment();
    }
    setState(244);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx)) {
    case 1: {
      setState(234);
      fieldDecl();
      break;
    }

    case 2: {
      setState(235);
      virtualDecl();
      break;
    }

    case 3: {
      setState(236);
      aliasDecl();
      break;
    }

    case 4: {
      setState(237);
      bitsBlock();
      break;
    }

    case 5: {
      setState(238);
      variantDecl();
      break;
    }

    case 6: {
      setState(239);
      conditionalDecl();
      break;
    }

    case 7: {
      setState(240);
      blockDecl();
      break;
    }

    case 8: {
      setState(241);
      atDecl();
      break;
    }

    case 9: {
      setState(242);
      alignDecl();
      break;
    }

    case 10: {
      setState(243);
      callbackUse();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VirtualDeclContext ------------------------------------------------------------------

EmbXParser::VirtualDeclContext::VirtualDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::VirtualDeclContext::LET() {
  return getToken(EmbXParser::LET, 0);
}

tree::TerminalNode* EmbXParser::VirtualDeclContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

EmbXParser::ExprContext* EmbXParser::VirtualDeclContext::expr() {
  return getRuleContext<EmbXParser::ExprContext>(0);
}

EmbXParser::AttributesContext* EmbXParser::VirtualDeclContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}


size_t EmbXParser::VirtualDeclContext::getRuleIndex() const {
  return EmbXParser::RuleVirtualDecl;
}


std::any EmbXParser::VirtualDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitVirtualDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::VirtualDeclContext* EmbXParser::virtualDecl() {
  VirtualDeclContext *_localctx = _tracker.createInstance<VirtualDeclContext>(_ctx, getState());
  enterRule(_localctx, 28, EmbXParser::RuleVirtualDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(247);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(246);
      attributes();
    }
    setState(249);
    match(EmbXParser::LET);
    setState(250);
    match(EmbXParser::ID);
    setState(251);
    match(EmbXParser::T__1);
    setState(252);
    expr(0);
    setState(253);
    match(EmbXParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AliasDeclContext ------------------------------------------------------------------

EmbXParser::AliasDeclContext::AliasDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::AliasDeclContext::ALIAS() {
  return getToken(EmbXParser::ALIAS, 0);
}

std::vector<tree::TerminalNode *> EmbXParser::AliasDeclContext::ID() {
  return getTokens(EmbXParser::ID);
}

tree::TerminalNode* EmbXParser::AliasDeclContext::ID(size_t i) {
  return getToken(EmbXParser::ID, i);
}

EmbXParser::AttributesContext* EmbXParser::AliasDeclContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}


size_t EmbXParser::AliasDeclContext::getRuleIndex() const {
  return EmbXParser::RuleAliasDecl;
}


std::any EmbXParser::AliasDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitAliasDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::AliasDeclContext* EmbXParser::aliasDecl() {
  AliasDeclContext *_localctx = _tracker.createInstance<AliasDeclContext>(_ctx, getState());
  enterRule(_localctx, 30, EmbXParser::RuleAliasDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(256);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(255);
      attributes();
    }
    setState(258);
    match(EmbXParser::ALIAS);
    setState(259);
    match(EmbXParser::ID);
    setState(260);
    match(EmbXParser::T__1);
    setState(261);
    match(EmbXParser::ID);
    setState(262);
    match(EmbXParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FieldDeclContext ------------------------------------------------------------------

EmbXParser::FieldDeclContext::FieldDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::FieldDeclContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

EmbXParser::TypeRefContext* EmbXParser::FieldDeclContext::typeRef() {
  return getRuleContext<EmbXParser::TypeRefContext>(0);
}

EmbXParser::AttributesContext* EmbXParser::FieldDeclContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}

std::vector<EmbXParser::FieldModifierContext *> EmbXParser::FieldDeclContext::fieldModifier() {
  return getRuleContexts<EmbXParser::FieldModifierContext>();
}

EmbXParser::FieldModifierContext* EmbXParser::FieldDeclContext::fieldModifier(size_t i) {
  return getRuleContext<EmbXParser::FieldModifierContext>(i);
}

EmbXParser::TransformContext* EmbXParser::FieldDeclContext::transform() {
  return getRuleContext<EmbXParser::TransformContext>(0);
}

EmbXParser::LiteralContext* EmbXParser::FieldDeclContext::literal() {
  return getRuleContext<EmbXParser::LiteralContext>(0);
}


size_t EmbXParser::FieldDeclContext::getRuleIndex() const {
  return EmbXParser::RuleFieldDecl;
}


std::any EmbXParser::FieldDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitFieldDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::FieldDeclContext* EmbXParser::fieldDecl() {
  FieldDeclContext *_localctx = _tracker.createInstance<FieldDeclContext>(_ctx, getState());
  enterRule(_localctx, 32, EmbXParser::RuleFieldDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(265);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(264);
      attributes();
    }
    setState(267);
    match(EmbXParser::ID);
    setState(268);
    match(EmbXParser::T__2);
    setState(269);
    typeRef();
    setState(273);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 4035225266123965056) != 0)) {
      setState(270);
      fieldModifier();
      setState(275);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(277);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::TRANSFORM) {
      setState(276);
      transform();
    }
    setState(281);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__1) {
      setState(279);
      match(EmbXParser::T__1);
      setState(280);
      literal();
    }
    setState(283);
    match(EmbXParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FieldModifierContext ------------------------------------------------------------------

EmbXParser::FieldModifierContext::FieldModifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EmbXParser::EndianContext* EmbXParser::FieldModifierContext::endian() {
  return getRuleContext<EmbXParser::EndianContext>(0);
}

EmbXParser::ExprContext* EmbXParser::FieldModifierContext::expr() {
  return getRuleContext<EmbXParser::ExprContext>(0);
}

tree::TerminalNode* EmbXParser::FieldModifierContext::INT() {
  return getToken(EmbXParser::INT, 0);
}


size_t EmbXParser::FieldModifierContext::getRuleIndex() const {
  return EmbXParser::RuleFieldModifier;
}


std::any EmbXParser::FieldModifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitFieldModifier(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::FieldModifierContext* EmbXParser::fieldModifier() {
  FieldModifierContext *_localctx = _tracker.createInstance<FieldModifierContext>(_ctx, getState());
  enterRule(_localctx, 34, EmbXParser::RuleFieldModifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(293);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EmbXParser::LITTLE:
      case EmbXParser::BIG:
      case EmbXParser::NATIVE: {
        enterOuterAlt(_localctx, 1);
        setState(285);
        endian();
        break;
      }

      case EmbXParser::T__6: {
        enterOuterAlt(_localctx, 2);
        setState(286);
        match(EmbXParser::T__6);
        setState(287);
        expr(0);
        setState(288);
        match(EmbXParser::T__7);
        break;
      }

      case EmbXParser::T__8: {
        enterOuterAlt(_localctx, 3);
        setState(290);
        match(EmbXParser::T__8);
        setState(291);
        match(EmbXParser::INT);
        setState(292);
        match(EmbXParser::T__9);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TransformContext ------------------------------------------------------------------

EmbXParser::TransformContext::TransformContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::TransformContext::TRANSFORM() {
  return getToken(EmbXParser::TRANSFORM, 0);
}

tree::TerminalNode* EmbXParser::TransformContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

std::vector<EmbXParser::ExprContext *> EmbXParser::TransformContext::expr() {
  return getRuleContexts<EmbXParser::ExprContext>();
}

EmbXParser::ExprContext* EmbXParser::TransformContext::expr(size_t i) {
  return getRuleContext<EmbXParser::ExprContext>(i);
}


size_t EmbXParser::TransformContext::getRuleIndex() const {
  return EmbXParser::RuleTransform;
}


std::any EmbXParser::TransformContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitTransform(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::TransformContext* EmbXParser::transform() {
  TransformContext *_localctx = _tracker.createInstance<TransformContext>(_ctx, getState());
  enterRule(_localctx, 36, EmbXParser::RuleTransform);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(295);
    match(EmbXParser::TRANSFORM);
    setState(296);
    match(EmbXParser::ID);
    setState(297);
    match(EmbXParser::T__8);
    setState(298);
    expr(0);
    setState(303);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EmbXParser::T__4) {
      setState(299);
      match(EmbXParser::T__4);
      setState(300);
      expr(0);
      setState(305);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(306);
    match(EmbXParser::T__9);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BitsBlockContext ------------------------------------------------------------------

EmbXParser::BitsBlockContext::BitsBlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::BitsBlockContext::BITS() {
  return getToken(EmbXParser::BITS, 0);
}

EmbXParser::AttributesContext* EmbXParser::BitsBlockContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}

std::vector<EmbXParser::BitFieldContext *> EmbXParser::BitsBlockContext::bitField() {
  return getRuleContexts<EmbXParser::BitFieldContext>();
}

EmbXParser::BitFieldContext* EmbXParser::BitsBlockContext::bitField(size_t i) {
  return getRuleContext<EmbXParser::BitFieldContext>(i);
}


size_t EmbXParser::BitsBlockContext::getRuleIndex() const {
  return EmbXParser::RuleBitsBlock;
}


std::any EmbXParser::BitsBlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitBitsBlock(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::BitsBlockContext* EmbXParser::bitsBlock() {
  BitsBlockContext *_localctx = _tracker.createInstance<BitsBlockContext>(_ctx, getState());
  enterRule(_localctx, 38, EmbXParser::RuleBitsBlock);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(309);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(308);
      attributes();
    }
    setState(311);
    match(EmbXParser::BITS);
    setState(312);
    match(EmbXParser::T__3);
    setState(316);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EmbXParser::ID

    || _la == EmbXParser::DOC_COMMENT) {
      setState(313);
      bitField();
      setState(318);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(319);
    match(EmbXParser::T__5);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BitFieldContext ------------------------------------------------------------------

EmbXParser::BitFieldContext::BitFieldContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::BitFieldContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

EmbXParser::TypeRefContext* EmbXParser::BitFieldContext::typeRef() {
  return getRuleContext<EmbXParser::TypeRefContext>(0);
}

tree::TerminalNode* EmbXParser::BitFieldContext::INT() {
  return getToken(EmbXParser::INT, 0);
}

EmbXParser::DocCommentContext* EmbXParser::BitFieldContext::docComment() {
  return getRuleContext<EmbXParser::DocCommentContext>(0);
}

std::vector<EmbXParser::FieldModifierContext *> EmbXParser::BitFieldContext::fieldModifier() {
  return getRuleContexts<EmbXParser::FieldModifierContext>();
}

EmbXParser::FieldModifierContext* EmbXParser::BitFieldContext::fieldModifier(size_t i) {
  return getRuleContext<EmbXParser::FieldModifierContext>(i);
}

EmbXParser::LiteralContext* EmbXParser::BitFieldContext::literal() {
  return getRuleContext<EmbXParser::LiteralContext>(0);
}


size_t EmbXParser::BitFieldContext::getRuleIndex() const {
  return EmbXParser::RuleBitField;
}


std::any EmbXParser::BitFieldContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitBitField(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::BitFieldContext* EmbXParser::bitField() {
  BitFieldContext *_localctx = _tracker.createInstance<BitFieldContext>(_ctx, getState());
  enterRule(_localctx, 40, EmbXParser::RuleBitField);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(322);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::DOC_COMMENT) {
      setState(321);
      docComment();
    }
    setState(324);
    match(EmbXParser::ID);
    setState(325);
    match(EmbXParser::T__2);
    setState(326);
    typeRef();
    setState(327);
    match(EmbXParser::T__8);
    setState(328);
    match(EmbXParser::INT);
    setState(329);
    match(EmbXParser::T__9);
    setState(333);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 4035225266123965056) != 0)) {
      setState(330);
      fieldModifier();
      setState(335);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(338);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__1) {
      setState(336);
      match(EmbXParser::T__1);
      setState(337);
      literal();
    }
    setState(340);
    match(EmbXParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConditionalDeclContext ------------------------------------------------------------------

EmbXParser::ConditionalDeclContext::ConditionalDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::ConditionalDeclContext::IF() {
  return getToken(EmbXParser::IF, 0);
}

EmbXParser::ExprContext* EmbXParser::ConditionalDeclContext::expr() {
  return getRuleContext<EmbXParser::ExprContext>(0);
}

EmbXParser::AttributesContext* EmbXParser::ConditionalDeclContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}

std::vector<EmbXParser::MemberContext *> EmbXParser::ConditionalDeclContext::member() {
  return getRuleContexts<EmbXParser::MemberContext>();
}

EmbXParser::MemberContext* EmbXParser::ConditionalDeclContext::member(size_t i) {
  return getRuleContext<EmbXParser::MemberContext>(i);
}

tree::TerminalNode* EmbXParser::ConditionalDeclContext::ELSE() {
  return getToken(EmbXParser::ELSE, 0);
}


size_t EmbXParser::ConditionalDeclContext::getRuleIndex() const {
  return EmbXParser::RuleConditionalDecl;
}


std::any EmbXParser::ConditionalDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitConditionalDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::ConditionalDeclContext* EmbXParser::conditionalDecl() {
  ConditionalDeclContext *_localctx = _tracker.createInstance<ConditionalDeclContext>(_ctx, getState());
  enterRule(_localctx, 42, EmbXParser::RuleConditionalDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(343);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(342);
      attributes();
    }
    setState(345);
    match(EmbXParser::IF);
    setState(346);
    match(EmbXParser::T__8);
    setState(347);
    expr(0);
    setState(348);
    match(EmbXParser::T__9);
    setState(349);
    match(EmbXParser::T__3);
    setState(353);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 2444420506976384) != 0) || _la == EmbXParser::ID

    || _la == EmbXParser::DOC_COMMENT) {
      setState(350);
      member();
      setState(355);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(356);
    match(EmbXParser::T__5);
    setState(366);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::ELSE) {
      setState(357);
      match(EmbXParser::ELSE);
      setState(358);
      match(EmbXParser::T__3);
      setState(362);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 2444420506976384) != 0) || _la == EmbXParser::ID

      || _la == EmbXParser::DOC_COMMENT) {
        setState(359);
        member();
        setState(364);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(365);
      match(EmbXParser::T__5);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VariantDeclContext ------------------------------------------------------------------

EmbXParser::VariantDeclContext::VariantDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::VariantDeclContext::VARIANT() {
  return getToken(EmbXParser::VARIANT, 0);
}

tree::TerminalNode* EmbXParser::VariantDeclContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

tree::TerminalNode* EmbXParser::VariantDeclContext::BY() {
  return getToken(EmbXParser::BY, 0);
}

EmbXParser::ExprContext* EmbXParser::VariantDeclContext::expr() {
  return getRuleContext<EmbXParser::ExprContext>(0);
}

EmbXParser::AttributesContext* EmbXParser::VariantDeclContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}

std::vector<EmbXParser::VariantCaseContext *> EmbXParser::VariantDeclContext::variantCase() {
  return getRuleContexts<EmbXParser::VariantCaseContext>();
}

EmbXParser::VariantCaseContext* EmbXParser::VariantDeclContext::variantCase(size_t i) {
  return getRuleContext<EmbXParser::VariantCaseContext>(i);
}

EmbXParser::VariantDefaultContext* EmbXParser::VariantDeclContext::variantDefault() {
  return getRuleContext<EmbXParser::VariantDefaultContext>(0);
}


size_t EmbXParser::VariantDeclContext::getRuleIndex() const {
  return EmbXParser::RuleVariantDecl;
}


std::any EmbXParser::VariantDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitVariantDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::VariantDeclContext* EmbXParser::variantDecl() {
  VariantDeclContext *_localctx = _tracker.createInstance<VariantDeclContext>(_ctx, getState());
  enterRule(_localctx, 44, EmbXParser::RuleVariantDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(369);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(368);
      attributes();
    }
    setState(371);
    match(EmbXParser::VARIANT);
    setState(372);
    match(EmbXParser::ID);
    setState(373);
    match(EmbXParser::BY);
    setState(374);
    expr(0);
    setState(375);
    match(EmbXParser::T__3);
    setState(377); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(376);
      variantCase();
      setState(379); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 567453553048685056) != 0) || ((((_la - 74) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 74)) & 31) != 0));
    setState(382);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::DEFAULT) {
      setState(381);
      variantDefault();
    }
    setState(384);
    match(EmbXParser::T__5);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VariantCaseContext ------------------------------------------------------------------

EmbXParser::VariantCaseContext::VariantCaseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EmbXParser::ExprContext* EmbXParser::VariantCaseContext::expr() {
  return getRuleContext<EmbXParser::ExprContext>(0);
}

EmbXParser::VariantBodyContext* EmbXParser::VariantCaseContext::variantBody() {
  return getRuleContext<EmbXParser::VariantBodyContext>(0);
}


size_t EmbXParser::VariantCaseContext::getRuleIndex() const {
  return EmbXParser::RuleVariantCase;
}


std::any EmbXParser::VariantCaseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitVariantCase(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::VariantCaseContext* EmbXParser::variantCase() {
  VariantCaseContext *_localctx = _tracker.createInstance<VariantCaseContext>(_ctx, getState());
  enterRule(_localctx, 46, EmbXParser::RuleVariantCase);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(386);
    expr(0);
    setState(387);
    match(EmbXParser::T__2);
    setState(388);
    variantBody();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VariantDefaultContext ------------------------------------------------------------------

EmbXParser::VariantDefaultContext::VariantDefaultContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::VariantDefaultContext::DEFAULT() {
  return getToken(EmbXParser::DEFAULT, 0);
}

EmbXParser::VariantBodyContext* EmbXParser::VariantDefaultContext::variantBody() {
  return getRuleContext<EmbXParser::VariantBodyContext>(0);
}


size_t EmbXParser::VariantDefaultContext::getRuleIndex() const {
  return EmbXParser::RuleVariantDefault;
}


std::any EmbXParser::VariantDefaultContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitVariantDefault(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::VariantDefaultContext* EmbXParser::variantDefault() {
  VariantDefaultContext *_localctx = _tracker.createInstance<VariantDefaultContext>(_ctx, getState());
  enterRule(_localctx, 48, EmbXParser::RuleVariantDefault);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(390);
    match(EmbXParser::DEFAULT);
    setState(391);
    match(EmbXParser::T__2);
    setState(392);
    variantBody();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VariantBodyContext ------------------------------------------------------------------

EmbXParser::VariantBodyContext::VariantBodyContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EmbXParser::TypeRefContext* EmbXParser::VariantBodyContext::typeRef() {
  return getRuleContext<EmbXParser::TypeRefContext>(0);
}

std::vector<EmbXParser::MemberContext *> EmbXParser::VariantBodyContext::member() {
  return getRuleContexts<EmbXParser::MemberContext>();
}

EmbXParser::MemberContext* EmbXParser::VariantBodyContext::member(size_t i) {
  return getRuleContext<EmbXParser::MemberContext>(i);
}


size_t EmbXParser::VariantBodyContext::getRuleIndex() const {
  return EmbXParser::RuleVariantBody;
}


std::any EmbXParser::VariantBodyContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitVariantBody(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::VariantBodyContext* EmbXParser::variantBody() {
  VariantBodyContext *_localctx = _tracker.createInstance<VariantBodyContext>(_ctx, getState());
  enterRule(_localctx, 50, EmbXParser::RuleVariantBody);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(405);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EmbXParser::U8:
      case EmbXParser::I8:
      case EmbXParser::U16:
      case EmbXParser::I16:
      case EmbXParser::U32:
      case EmbXParser::I32:
      case EmbXParser::U64:
      case EmbXParser::I64:
      case EmbXParser::F32:
      case EmbXParser::F64:
      case EmbXParser::BYTES:
      case EmbXParser::STRINGTYPE:
      case EmbXParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(394);
        typeRef();
        setState(395);
        match(EmbXParser::T__0);
        break;
      }

      case EmbXParser::T__3: {
        enterOuterAlt(_localctx, 2);
        setState(397);
        match(EmbXParser::T__3);
        setState(401);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 2444420506976384) != 0) || _la == EmbXParser::ID

        || _la == EmbXParser::DOC_COMMENT) {
          setState(398);
          member();
          setState(403);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
        setState(404);
        match(EmbXParser::T__5);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockDeclContext ------------------------------------------------------------------

EmbXParser::BlockDeclContext::BlockDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::BlockDeclContext::BLOCK() {
  return getToken(EmbXParser::BLOCK, 0);
}

EmbXParser::ExprContext* EmbXParser::BlockDeclContext::expr() {
  return getRuleContext<EmbXParser::ExprContext>(0);
}

EmbXParser::AttributesContext* EmbXParser::BlockDeclContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}

tree::TerminalNode* EmbXParser::BlockDeclContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

std::vector<EmbXParser::MemberContext *> EmbXParser::BlockDeclContext::member() {
  return getRuleContexts<EmbXParser::MemberContext>();
}

EmbXParser::MemberContext* EmbXParser::BlockDeclContext::member(size_t i) {
  return getRuleContext<EmbXParser::MemberContext>(i);
}


size_t EmbXParser::BlockDeclContext::getRuleIndex() const {
  return EmbXParser::RuleBlockDecl;
}


std::any EmbXParser::BlockDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitBlockDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::BlockDeclContext* EmbXParser::blockDecl() {
  BlockDeclContext *_localctx = _tracker.createInstance<BlockDeclContext>(_ctx, getState());
  enterRule(_localctx, 52, EmbXParser::RuleBlockDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(408);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(407);
      attributes();
    }
    setState(410);
    match(EmbXParser::BLOCK);
    setState(412);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::ID) {
      setState(411);
      match(EmbXParser::ID);
    }
    setState(414);
    match(EmbXParser::T__6);
    setState(415);
    expr(0);
    setState(416);
    match(EmbXParser::T__7);
    setState(417);
    match(EmbXParser::T__3);
    setState(421);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 2444420506976384) != 0) || _la == EmbXParser::ID

    || _la == EmbXParser::DOC_COMMENT) {
      setState(418);
      member();
      setState(423);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(424);
    match(EmbXParser::T__5);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AtDeclContext ------------------------------------------------------------------

EmbXParser::AtDeclContext::AtDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::AtDeclContext::ATCALL() {
  return getToken(EmbXParser::ATCALL, 0);
}

EmbXParser::ExprContext* EmbXParser::AtDeclContext::expr() {
  return getRuleContext<EmbXParser::ExprContext>(0);
}

EmbXParser::AttributesContext* EmbXParser::AtDeclContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}

std::vector<EmbXParser::MemberContext *> EmbXParser::AtDeclContext::member() {
  return getRuleContexts<EmbXParser::MemberContext>();
}

EmbXParser::MemberContext* EmbXParser::AtDeclContext::member(size_t i) {
  return getRuleContext<EmbXParser::MemberContext>(i);
}


size_t EmbXParser::AtDeclContext::getRuleIndex() const {
  return EmbXParser::RuleAtDecl;
}


std::any EmbXParser::AtDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitAtDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::AtDeclContext* EmbXParser::atDecl() {
  AtDeclContext *_localctx = _tracker.createInstance<AtDeclContext>(_ctx, getState());
  enterRule(_localctx, 54, EmbXParser::RuleAtDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(427);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(426);
      attributes();
    }
    setState(429);
    match(EmbXParser::ATCALL);
    setState(430);
    match(EmbXParser::T__8);
    setState(431);
    expr(0);
    setState(432);
    match(EmbXParser::T__9);
    setState(433);
    match(EmbXParser::T__3);
    setState(437);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 2444420506976384) != 0) || _la == EmbXParser::ID

    || _la == EmbXParser::DOC_COMMENT) {
      setState(434);
      member();
      setState(439);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(440);
    match(EmbXParser::T__5);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AlignDeclContext ------------------------------------------------------------------

EmbXParser::AlignDeclContext::AlignDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::AlignDeclContext::ALIGN() {
  return getToken(EmbXParser::ALIGN, 0);
}

EmbXParser::ExprContext* EmbXParser::AlignDeclContext::expr() {
  return getRuleContext<EmbXParser::ExprContext>(0);
}

EmbXParser::AttributesContext* EmbXParser::AlignDeclContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}


size_t EmbXParser::AlignDeclContext::getRuleIndex() const {
  return EmbXParser::RuleAlignDecl;
}


std::any EmbXParser::AlignDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitAlignDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::AlignDeclContext* EmbXParser::alignDecl() {
  AlignDeclContext *_localctx = _tracker.createInstance<AlignDeclContext>(_ctx, getState());
  enterRule(_localctx, 56, EmbXParser::RuleAlignDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(443);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(442);
      attributes();
    }
    setState(445);
    match(EmbXParser::ALIGN);
    setState(446);
    match(EmbXParser::T__8);
    setState(447);
    expr(0);
    setState(448);
    match(EmbXParser::T__9);
    setState(449);
    match(EmbXParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CallbackDeclContext ------------------------------------------------------------------

EmbXParser::CallbackDeclContext::CallbackDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::CallbackDeclContext::CALLBACK() {
  return getToken(EmbXParser::CALLBACK, 0);
}

std::vector<tree::TerminalNode *> EmbXParser::CallbackDeclContext::ID() {
  return getTokens(EmbXParser::ID);
}

tree::TerminalNode* EmbXParser::CallbackDeclContext::ID(size_t i) {
  return getToken(EmbXParser::ID, i);
}

EmbXParser::AttributesContext* EmbXParser::CallbackDeclContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}


size_t EmbXParser::CallbackDeclContext::getRuleIndex() const {
  return EmbXParser::RuleCallbackDecl;
}


std::any EmbXParser::CallbackDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitCallbackDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::CallbackDeclContext* EmbXParser::callbackDecl() {
  CallbackDeclContext *_localctx = _tracker.createInstance<CallbackDeclContext>(_ctx, getState());
  enterRule(_localctx, 58, EmbXParser::RuleCallbackDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(452);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(451);
      attributes();
    }
    setState(454);
    match(EmbXParser::CALLBACK);
    setState(455);
    match(EmbXParser::ID);
    setState(461);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__8) {
      setState(456);
      match(EmbXParser::T__8);
      setState(458);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == EmbXParser::ID) {
        setState(457);
        match(EmbXParser::ID);
      }
      setState(460);
      match(EmbXParser::T__9);
    }
    setState(463);
    match(EmbXParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParameterDeclContext ------------------------------------------------------------------

EmbXParser::ParameterDeclContext::ParameterDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::ParameterDeclContext::PARAM() {
  return getToken(EmbXParser::PARAM, 0);
}

tree::TerminalNode* EmbXParser::ParameterDeclContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

EmbXParser::TypeRefContext* EmbXParser::ParameterDeclContext::typeRef() {
  return getRuleContext<EmbXParser::TypeRefContext>(0);
}

EmbXParser::AttributesContext* EmbXParser::ParameterDeclContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}


size_t EmbXParser::ParameterDeclContext::getRuleIndex() const {
  return EmbXParser::RuleParameterDecl;
}


std::any EmbXParser::ParameterDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitParameterDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::ParameterDeclContext* EmbXParser::parameterDecl() {
  ParameterDeclContext *_localctx = _tracker.createInstance<ParameterDeclContext>(_ctx, getState());
  enterRule(_localctx, 60, EmbXParser::RuleParameterDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(466);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(465);
      attributes();
    }
    setState(468);
    match(EmbXParser::PARAM);
    setState(469);
    match(EmbXParser::ID);
    setState(470);
    match(EmbXParser::T__2);
    setState(471);
    typeRef();
    setState(472);
    match(EmbXParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CallbackUseContext ------------------------------------------------------------------

EmbXParser::CallbackUseContext::CallbackUseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::CallbackUseContext::CALLBACK() {
  return getToken(EmbXParser::CALLBACK, 0);
}

tree::TerminalNode* EmbXParser::CallbackUseContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

EmbXParser::AttributesContext* EmbXParser::CallbackUseContext::attributes() {
  return getRuleContext<EmbXParser::AttributesContext>(0);
}

EmbXParser::ArgListContext* EmbXParser::CallbackUseContext::argList() {
  return getRuleContext<EmbXParser::ArgListContext>(0);
}


size_t EmbXParser::CallbackUseContext::getRuleIndex() const {
  return EmbXParser::RuleCallbackUse;
}


std::any EmbXParser::CallbackUseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitCallbackUse(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::CallbackUseContext* EmbXParser::callbackUse() {
  CallbackUseContext *_localctx = _tracker.createInstance<CallbackUseContext>(_ctx, getState());
  enterRule(_localctx, 62, EmbXParser::RuleCallbackUse);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(475);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__6) {
      setState(474);
      attributes();
    }
    setState(477);
    match(EmbXParser::CALLBACK);
    setState(478);
    match(EmbXParser::ID);
    setState(479);
    match(EmbXParser::T__8);
    setState(481);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 567453553048685056) != 0) || ((((_la - 74) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 74)) & 31) != 0)) {
      setState(480);
      argList();
    }
    setState(483);
    match(EmbXParser::T__9);
    setState(484);
    match(EmbXParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ArgListContext ------------------------------------------------------------------

EmbXParser::ArgListContext::ArgListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EmbXParser::ExprContext *> EmbXParser::ArgListContext::expr() {
  return getRuleContexts<EmbXParser::ExprContext>();
}

EmbXParser::ExprContext* EmbXParser::ArgListContext::expr(size_t i) {
  return getRuleContext<EmbXParser::ExprContext>(i);
}


size_t EmbXParser::ArgListContext::getRuleIndex() const {
  return EmbXParser::RuleArgList;
}


std::any EmbXParser::ArgListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitArgList(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::ArgListContext* EmbXParser::argList() {
  ArgListContext *_localctx = _tracker.createInstance<ArgListContext>(_ctx, getState());
  enterRule(_localctx, 64, EmbXParser::RuleArgList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(486);
    expr(0);
    setState(491);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EmbXParser::T__4) {
      setState(487);
      match(EmbXParser::T__4);
      setState(488);
      expr(0);
      setState(493);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DocCommentContext ------------------------------------------------------------------

EmbXParser::DocCommentContext::DocCommentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> EmbXParser::DocCommentContext::DOC_COMMENT() {
  return getTokens(EmbXParser::DOC_COMMENT);
}

tree::TerminalNode* EmbXParser::DocCommentContext::DOC_COMMENT(size_t i) {
  return getToken(EmbXParser::DOC_COMMENT, i);
}


size_t EmbXParser::DocCommentContext::getRuleIndex() const {
  return EmbXParser::RuleDocComment;
}


std::any EmbXParser::DocCommentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitDocComment(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::DocCommentContext* EmbXParser::docComment() {
  DocCommentContext *_localctx = _tracker.createInstance<DocCommentContext>(_ctx, getState());
  enterRule(_localctx, 66, EmbXParser::RuleDocComment);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(495); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(494);
      match(EmbXParser::DOC_COMMENT);
      setState(497); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while (_la == EmbXParser::DOC_COMMENT);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttributeDeclContext ------------------------------------------------------------------

EmbXParser::AttributeDeclContext::AttributeDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::AttributeDeclContext::ATTRIBUTE() {
  return getToken(EmbXParser::ATTRIBUTE, 0);
}

tree::TerminalNode* EmbXParser::AttributeDeclContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

EmbXParser::AttributeTypeContext* EmbXParser::AttributeDeclContext::attributeType() {
  return getRuleContext<EmbXParser::AttributeTypeContext>(0);
}


size_t EmbXParser::AttributeDeclContext::getRuleIndex() const {
  return EmbXParser::RuleAttributeDecl;
}


std::any EmbXParser::AttributeDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitAttributeDecl(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::AttributeDeclContext* EmbXParser::attributeDecl() {
  AttributeDeclContext *_localctx = _tracker.createInstance<AttributeDeclContext>(_ctx, getState());
  enterRule(_localctx, 68, EmbXParser::RuleAttributeDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(499);
    match(EmbXParser::ATTRIBUTE);
    setState(500);
    match(EmbXParser::ID);
    setState(503);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__2) {
      setState(501);
      match(EmbXParser::T__2);
      setState(502);
      attributeType();
    }
    setState(505);
    match(EmbXParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttributeTypeContext ------------------------------------------------------------------

EmbXParser::AttributeTypeContext::AttributeTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::AttributeTypeContext::U8() {
  return getToken(EmbXParser::U8, 0);
}

tree::TerminalNode* EmbXParser::AttributeTypeContext::I8() {
  return getToken(EmbXParser::I8, 0);
}

tree::TerminalNode* EmbXParser::AttributeTypeContext::U16() {
  return getToken(EmbXParser::U16, 0);
}

tree::TerminalNode* EmbXParser::AttributeTypeContext::I16() {
  return getToken(EmbXParser::I16, 0);
}

tree::TerminalNode* EmbXParser::AttributeTypeContext::U32() {
  return getToken(EmbXParser::U32, 0);
}

tree::TerminalNode* EmbXParser::AttributeTypeContext::I32() {
  return getToken(EmbXParser::I32, 0);
}

tree::TerminalNode* EmbXParser::AttributeTypeContext::U64() {
  return getToken(EmbXParser::U64, 0);
}

tree::TerminalNode* EmbXParser::AttributeTypeContext::I64() {
  return getToken(EmbXParser::I64, 0);
}

tree::TerminalNode* EmbXParser::AttributeTypeContext::F32() {
  return getToken(EmbXParser::F32, 0);
}

tree::TerminalNode* EmbXParser::AttributeTypeContext::F64() {
  return getToken(EmbXParser::F64, 0);
}

tree::TerminalNode* EmbXParser::AttributeTypeContext::STRINGTYPE() {
  return getToken(EmbXParser::STRINGTYPE, 0);
}


size_t EmbXParser::AttributeTypeContext::getRuleIndex() const {
  return EmbXParser::RuleAttributeType;
}


std::any EmbXParser::AttributeTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitAttributeType(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::AttributeTypeContext* EmbXParser::attributeType() {
  AttributeTypeContext *_localctx = _tracker.createInstance<AttributeTypeContext>(_ctx, getState());
  enterRule(_localctx, 70, EmbXParser::RuleAttributeType);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(507);
    _la = _input->LA(1);
    if (!(((((_la - 62) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 62)) & 3071) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttributesContext ------------------------------------------------------------------

EmbXParser::AttributesContext::AttributesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EmbXParser::AttributeEntryContext *> EmbXParser::AttributesContext::attributeEntry() {
  return getRuleContexts<EmbXParser::AttributeEntryContext>();
}

EmbXParser::AttributeEntryContext* EmbXParser::AttributesContext::attributeEntry(size_t i) {
  return getRuleContext<EmbXParser::AttributeEntryContext>(i);
}


size_t EmbXParser::AttributesContext::getRuleIndex() const {
  return EmbXParser::RuleAttributes;
}


std::any EmbXParser::AttributesContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitAttributes(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::AttributesContext* EmbXParser::attributes() {
  AttributesContext *_localctx = _tracker.createInstance<AttributesContext>(_ctx, getState());
  enterRule(_localctx, 72, EmbXParser::RuleAttributes);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(509);
    match(EmbXParser::T__6);
    setState(510);
    attributeEntry();
    setState(515);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 59, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(511);
        match(EmbXParser::T__4);
        setState(512);
        attributeEntry(); 
      }
      setState(517);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 59, _ctx);
    }
    setState(519);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__4) {
      setState(518);
      match(EmbXParser::T__4);
    }
    setState(521);
    match(EmbXParser::T__7);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttributeEntryContext ------------------------------------------------------------------

EmbXParser::AttributeEntryContext::AttributeEntryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> EmbXParser::AttributeEntryContext::ID() {
  return getTokens(EmbXParser::ID);
}

tree::TerminalNode* EmbXParser::AttributeEntryContext::ID(size_t i) {
  return getToken(EmbXParser::ID, i);
}

EmbXParser::LiteralContext* EmbXParser::AttributeEntryContext::literal() {
  return getRuleContext<EmbXParser::LiteralContext>(0);
}

tree::TerminalNode* EmbXParser::AttributeEntryContext::TRUE() {
  return getToken(EmbXParser::TRUE, 0);
}

tree::TerminalNode* EmbXParser::AttributeEntryContext::FALSE() {
  return getToken(EmbXParser::FALSE, 0);
}


size_t EmbXParser::AttributeEntryContext::getRuleIndex() const {
  return EmbXParser::RuleAttributeEntry;
}


std::any EmbXParser::AttributeEntryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitAttributeEntry(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::AttributeEntryContext* EmbXParser::attributeEntry() {
  AttributeEntryContext *_localctx = _tracker.createInstance<AttributeEntryContext>(_ctx, getState());
  enterRule(_localctx, 74, EmbXParser::RuleAttributeEntry);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(523);
    match(EmbXParser::ID);
    setState(531);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__1) {
      setState(524);
      match(EmbXParser::T__1);
      setState(529);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case EmbXParser::T__10:
        case EmbXParser::HEX:
        case EmbXParser::FLOAT:
        case EmbXParser::INT:
        case EmbXParser::STRING: {
          setState(525);
          literal();
          break;
        }

        case EmbXParser::ID: {
          setState(526);
          match(EmbXParser::ID);
          break;
        }

        case EmbXParser::TRUE: {
          setState(527);
          match(EmbXParser::TRUE);
          break;
        }

        case EmbXParser::FALSE: {
          setState(528);
          match(EmbXParser::FALSE);
          break;
        }

      default:
        throw NoViableAltException(this);
      }
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeRefContext ------------------------------------------------------------------

EmbXParser::TypeRefContext::TypeRefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EmbXParser::QualifiedNameContext* EmbXParser::TypeRefContext::qualifiedName() {
  return getRuleContext<EmbXParser::QualifiedNameContext>(0);
}

EmbXParser::BaseTypeContext* EmbXParser::TypeRefContext::baseType() {
  return getRuleContext<EmbXParser::BaseTypeContext>(0);
}

EmbXParser::TypeSuffixContext* EmbXParser::TypeRefContext::typeSuffix() {
  return getRuleContext<EmbXParser::TypeSuffixContext>(0);
}

EmbXParser::TerminatedSequenceSuffixContext* EmbXParser::TypeRefContext::terminatedSequenceSuffix() {
  return getRuleContext<EmbXParser::TerminatedSequenceSuffixContext>(0);
}


size_t EmbXParser::TypeRefContext::getRuleIndex() const {
  return EmbXParser::RuleTypeRef;
}


std::any EmbXParser::TypeRefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitTypeRef(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::TypeRefContext* EmbXParser::typeRef() {
  TypeRefContext *_localctx = _tracker.createInstance<TypeRefContext>(_ctx, getState());
  enterRule(_localctx, 76, EmbXParser::RuleTypeRef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(535);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 63, _ctx)) {
    case 1: {
      setState(533);
      qualifiedName();
      break;
    }

    case 2: {
      setState(534);
      baseType();
      break;
    }

    default:
      break;
    }
    setState(538);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 64, _ctx)) {
    case 1: {
      setState(537);
      typeSuffix();
      break;
    }

    default:
      break;
    }
    setState(541);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::UNTIL) {
      setState(540);
      terminatedSequenceSuffix();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TerminatedSequenceSuffixContext ------------------------------------------------------------------

EmbXParser::TerminatedSequenceSuffixContext::TerminatedSequenceSuffixContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::TerminatedSequenceSuffixContext::UNTIL() {
  return getToken(EmbXParser::UNTIL, 0);
}

tree::TerminalNode* EmbXParser::TerminatedSequenceSuffixContext::MAX() {
  return getToken(EmbXParser::MAX, 0);
}

tree::TerminalNode* EmbXParser::TerminatedSequenceSuffixContext::INT() {
  return getToken(EmbXParser::INT, 0);
}

std::vector<tree::TerminalNode *> EmbXParser::TerminatedSequenceSuffixContext::HEX() {
  return getTokens(EmbXParser::HEX);
}

tree::TerminalNode* EmbXParser::TerminatedSequenceSuffixContext::HEX(size_t i) {
  return getToken(EmbXParser::HEX, i);
}


size_t EmbXParser::TerminatedSequenceSuffixContext::getRuleIndex() const {
  return EmbXParser::RuleTerminatedSequenceSuffix;
}


std::any EmbXParser::TerminatedSequenceSuffixContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitTerminatedSequenceSuffix(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::TerminatedSequenceSuffixContext* EmbXParser::terminatedSequenceSuffix() {
  TerminatedSequenceSuffixContext *_localctx = _tracker.createInstance<TerminatedSequenceSuffixContext>(_ctx, getState());
  enterRule(_localctx, 78, EmbXParser::RuleTerminatedSequenceSuffix);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(543);
    match(EmbXParser::UNTIL);
    setState(545); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(544);
      match(EmbXParser::HEX);
      setState(547); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while (_la == EmbXParser::HEX);
    setState(549);
    match(EmbXParser::MAX);
    setState(550);
    match(EmbXParser::INT);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeSuffixContext ------------------------------------------------------------------

EmbXParser::TypeSuffixContext::TypeSuffixContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::TypeSuffixContext::STAR() {
  return getToken(EmbXParser::STAR, 0);
}

EmbXParser::ExprContext* EmbXParser::TypeSuffixContext::expr() {
  return getRuleContext<EmbXParser::ExprContext>(0);
}


size_t EmbXParser::TypeSuffixContext::getRuleIndex() const {
  return EmbXParser::RuleTypeSuffix;
}


std::any EmbXParser::TypeSuffixContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitTypeSuffix(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::TypeSuffixContext* EmbXParser::typeSuffix() {
  TypeSuffixContext *_localctx = _tracker.createInstance<TypeSuffixContext>(_ctx, getState());
  enterRule(_localctx, 80, EmbXParser::RuleTypeSuffix);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(552);
    match(EmbXParser::T__6);
    setState(555);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EmbXParser::STAR: {
        setState(553);
        match(EmbXParser::STAR);
        break;
      }

      case EmbXParser::T__8:
      case EmbXParser::T__10:
      case EmbXParser::NEXT:
      case EmbXParser::TRUE:
      case EmbXParser::FALSE:
      case EmbXParser::SIZE_IN_BYTES:
      case EmbXParser::MIN_SIZE_IN_BYTES:
      case EmbXParser::MAX_SIZE_IN_BYTES:
      case EmbXParser::ID:
      case EmbXParser::HEX:
      case EmbXParser::FLOAT:
      case EmbXParser::INT:
      case EmbXParser::STRING: {
        setState(554);
        expr(0);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(557);
    match(EmbXParser::T__7);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BaseTypeContext ------------------------------------------------------------------

EmbXParser::BaseTypeContext::BaseTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EmbXParser::PrimitiveContext* EmbXParser::BaseTypeContext::primitive() {
  return getRuleContext<EmbXParser::PrimitiveContext>(0);
}

tree::TerminalNode* EmbXParser::BaseTypeContext::ID() {
  return getToken(EmbXParser::ID, 0);
}

tree::TerminalNode* EmbXParser::BaseTypeContext::BYTES() {
  return getToken(EmbXParser::BYTES, 0);
}

tree::TerminalNode* EmbXParser::BaseTypeContext::STRINGTYPE() {
  return getToken(EmbXParser::STRINGTYPE, 0);
}


size_t EmbXParser::BaseTypeContext::getRuleIndex() const {
  return EmbXParser::RuleBaseType;
}


std::any EmbXParser::BaseTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitBaseType(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::BaseTypeContext* EmbXParser::baseType() {
  BaseTypeContext *_localctx = _tracker.createInstance<BaseTypeContext>(_ctx, getState());
  enterRule(_localctx, 82, EmbXParser::RuleBaseType);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(563);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EmbXParser::U8:
      case EmbXParser::I8:
      case EmbXParser::U16:
      case EmbXParser::I16:
      case EmbXParser::U32:
      case EmbXParser::I32:
      case EmbXParser::U64:
      case EmbXParser::I64:
      case EmbXParser::F32:
      case EmbXParser::F64: {
        enterOuterAlt(_localctx, 1);
        setState(559);
        primitive();
        break;
      }

      case EmbXParser::ID: {
        enterOuterAlt(_localctx, 2);
        setState(560);
        match(EmbXParser::ID);
        break;
      }

      case EmbXParser::BYTES: {
        enterOuterAlt(_localctx, 3);
        setState(561);
        match(EmbXParser::BYTES);
        break;
      }

      case EmbXParser::STRINGTYPE: {
        enterOuterAlt(_localctx, 4);
        setState(562);
        match(EmbXParser::STRINGTYPE);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrimitiveContext ------------------------------------------------------------------

EmbXParser::PrimitiveContext::PrimitiveContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::PrimitiveContext::U8() {
  return getToken(EmbXParser::U8, 0);
}

tree::TerminalNode* EmbXParser::PrimitiveContext::I8() {
  return getToken(EmbXParser::I8, 0);
}

tree::TerminalNode* EmbXParser::PrimitiveContext::U16() {
  return getToken(EmbXParser::U16, 0);
}

tree::TerminalNode* EmbXParser::PrimitiveContext::I16() {
  return getToken(EmbXParser::I16, 0);
}

tree::TerminalNode* EmbXParser::PrimitiveContext::U32() {
  return getToken(EmbXParser::U32, 0);
}

tree::TerminalNode* EmbXParser::PrimitiveContext::I32() {
  return getToken(EmbXParser::I32, 0);
}

tree::TerminalNode* EmbXParser::PrimitiveContext::U64() {
  return getToken(EmbXParser::U64, 0);
}

tree::TerminalNode* EmbXParser::PrimitiveContext::I64() {
  return getToken(EmbXParser::I64, 0);
}

tree::TerminalNode* EmbXParser::PrimitiveContext::F32() {
  return getToken(EmbXParser::F32, 0);
}

tree::TerminalNode* EmbXParser::PrimitiveContext::F64() {
  return getToken(EmbXParser::F64, 0);
}


size_t EmbXParser::PrimitiveContext::getRuleIndex() const {
  return EmbXParser::RulePrimitive;
}


std::any EmbXParser::PrimitiveContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitPrimitive(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::PrimitiveContext* EmbXParser::primitive() {
  PrimitiveContext *_localctx = _tracker.createInstance<PrimitiveContext>(_ctx, getState());
  enterRule(_localctx, 84, EmbXParser::RulePrimitive);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(565);
    _la = _input->LA(1);
    if (!(((((_la - 62) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 62)) & 1023) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExprContext ------------------------------------------------------------------

EmbXParser::ExprContext::ExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EmbXParser::PrimaryContext* EmbXParser::ExprContext::primary() {
  return getRuleContext<EmbXParser::PrimaryContext>(0);
}

std::vector<EmbXParser::ExprContext *> EmbXParser::ExprContext::expr() {
  return getRuleContexts<EmbXParser::ExprContext>();
}

EmbXParser::ExprContext* EmbXParser::ExprContext::expr(size_t i) {
  return getRuleContext<EmbXParser::ExprContext>(i);
}

tree::TerminalNode* EmbXParser::ExprContext::STAR() {
  return getToken(EmbXParser::STAR, 0);
}


size_t EmbXParser::ExprContext::getRuleIndex() const {
  return EmbXParser::RuleExpr;
}


std::any EmbXParser::ExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitExpr(this);
  else
    return visitor->visitChildren(this);
}


EmbXParser::ExprContext* EmbXParser::expr() {
   return expr(0);
}

EmbXParser::ExprContext* EmbXParser::expr(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  EmbXParser::ExprContext *_localctx = _tracker.createInstance<ExprContext>(_ctx, parentState);
  EmbXParser::ExprContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 86;
  enterRecursionRule(_localctx, 86, EmbXParser::RuleExpr, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(571);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EmbXParser::T__8:
      case EmbXParser::NEXT:
      case EmbXParser::TRUE:
      case EmbXParser::FALSE:
      case EmbXParser::SIZE_IN_BYTES:
      case EmbXParser::MIN_SIZE_IN_BYTES:
      case EmbXParser::MAX_SIZE_IN_BYTES:
      case EmbXParser::ID:
      case EmbXParser::HEX:
      case EmbXParser::FLOAT:
      case EmbXParser::INT:
      case EmbXParser::STRING: {
        setState(568);
        primary();
        break;
      }

      case EmbXParser::T__10: {
        setState(569);
        match(EmbXParser::T__10);
        setState(570);
        expr(5);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(587);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 71, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(585);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 70, _ctx)) {
        case 1: {
          _localctx = _tracker.createInstance<ExprContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpr);
          setState(573);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(574);
          antlrcpp::downCast<ExprContext *>(_localctx)->op = _input->LT(1);
          _la = _input->LA(1);
          if (!((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 8400896) != 0))) {
            antlrcpp::downCast<ExprContext *>(_localctx)->op = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(575);
          expr(5);
          break;
        }

        case 2: {
          _localctx = _tracker.createInstance<ExprContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpr);
          setState(576);

          if (!(precpred(_ctx, 3))) throw FailedPredicateException(this, "precpred(_ctx, 3)");
          setState(577);
          antlrcpp::downCast<ExprContext *>(_localctx)->op = _input->LT(1);
          _la = _input->LA(1);
          if (!(_la == EmbXParser::T__10

          || _la == EmbXParser::T__13)) {
            antlrcpp::downCast<ExprContext *>(_localctx)->op = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(578);
          expr(4);
          break;
        }

        case 3: {
          _localctx = _tracker.createInstance<ExprContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpr);
          setState(579);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(580);
          antlrcpp::downCast<ExprContext *>(_localctx)->op = _input->LT(1);
          _la = _input->LA(1);
          if (!((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 2064384) != 0))) {
            antlrcpp::downCast<ExprContext *>(_localctx)->op = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(581);
          expr(3);
          break;
        }

        case 4: {
          _localctx = _tracker.createInstance<ExprContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpr);
          setState(582);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(583);
          antlrcpp::downCast<ExprContext *>(_localctx)->op = _input->LT(1);
          _la = _input->LA(1);
          if (!(_la == EmbXParser::T__20

          || _la == EmbXParser::T__21)) {
            antlrcpp::downCast<ExprContext *>(_localctx)->op = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(584);
          expr(2);
          break;
        }

        default:
          break;
        } 
      }
      setState(589);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 71, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- PrimaryContext ------------------------------------------------------------------

EmbXParser::PrimaryContext::PrimaryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::PrimaryContext::INT() {
  return getToken(EmbXParser::INT, 0);
}

tree::TerminalNode* EmbXParser::PrimaryContext::HEX() {
  return getToken(EmbXParser::HEX, 0);
}

tree::TerminalNode* EmbXParser::PrimaryContext::FLOAT() {
  return getToken(EmbXParser::FLOAT, 0);
}

tree::TerminalNode* EmbXParser::PrimaryContext::STRING() {
  return getToken(EmbXParser::STRING, 0);
}

tree::TerminalNode* EmbXParser::PrimaryContext::TRUE() {
  return getToken(EmbXParser::TRUE, 0);
}

tree::TerminalNode* EmbXParser::PrimaryContext::FALSE() {
  return getToken(EmbXParser::FALSE, 0);
}

tree::TerminalNode* EmbXParser::PrimaryContext::NEXT() {
  return getToken(EmbXParser::NEXT, 0);
}

tree::TerminalNode* EmbXParser::PrimaryContext::SIZE_IN_BYTES() {
  return getToken(EmbXParser::SIZE_IN_BYTES, 0);
}

tree::TerminalNode* EmbXParser::PrimaryContext::MIN_SIZE_IN_BYTES() {
  return getToken(EmbXParser::MIN_SIZE_IN_BYTES, 0);
}

tree::TerminalNode* EmbXParser::PrimaryContext::MAX_SIZE_IN_BYTES() {
  return getToken(EmbXParser::MAX_SIZE_IN_BYTES, 0);
}

EmbXParser::QualifiedNameContext* EmbXParser::PrimaryContext::qualifiedName() {
  return getRuleContext<EmbXParser::QualifiedNameContext>(0);
}

EmbXParser::ExprContext* EmbXParser::PrimaryContext::expr() {
  return getRuleContext<EmbXParser::ExprContext>(0);
}


size_t EmbXParser::PrimaryContext::getRuleIndex() const {
  return EmbXParser::RulePrimary;
}


std::any EmbXParser::PrimaryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitPrimary(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::PrimaryContext* EmbXParser::primary() {
  PrimaryContext *_localctx = _tracker.createInstance<PrimaryContext>(_ctx, getState());
  enterRule(_localctx, 88, EmbXParser::RulePrimary);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(605);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EmbXParser::INT: {
        enterOuterAlt(_localctx, 1);
        setState(590);
        match(EmbXParser::INT);
        break;
      }

      case EmbXParser::HEX: {
        enterOuterAlt(_localctx, 2);
        setState(591);
        match(EmbXParser::HEX);
        break;
      }

      case EmbXParser::FLOAT: {
        enterOuterAlt(_localctx, 3);
        setState(592);
        match(EmbXParser::FLOAT);
        break;
      }

      case EmbXParser::STRING: {
        enterOuterAlt(_localctx, 4);
        setState(593);
        match(EmbXParser::STRING);
        break;
      }

      case EmbXParser::TRUE: {
        enterOuterAlt(_localctx, 5);
        setState(594);
        match(EmbXParser::TRUE);
        break;
      }

      case EmbXParser::FALSE: {
        enterOuterAlt(_localctx, 6);
        setState(595);
        match(EmbXParser::FALSE);
        break;
      }

      case EmbXParser::NEXT: {
        enterOuterAlt(_localctx, 7);
        setState(596);
        match(EmbXParser::NEXT);
        break;
      }

      case EmbXParser::SIZE_IN_BYTES: {
        enterOuterAlt(_localctx, 8);
        setState(597);
        match(EmbXParser::SIZE_IN_BYTES);
        break;
      }

      case EmbXParser::MIN_SIZE_IN_BYTES: {
        enterOuterAlt(_localctx, 9);
        setState(598);
        match(EmbXParser::MIN_SIZE_IN_BYTES);
        break;
      }

      case EmbXParser::MAX_SIZE_IN_BYTES: {
        enterOuterAlt(_localctx, 10);
        setState(599);
        match(EmbXParser::MAX_SIZE_IN_BYTES);
        break;
      }

      case EmbXParser::ID: {
        enterOuterAlt(_localctx, 11);
        setState(600);
        qualifiedName();
        break;
      }

      case EmbXParser::T__8: {
        enterOuterAlt(_localctx, 12);
        setState(601);
        match(EmbXParser::T__8);
        setState(602);
        expr(0);
        setState(603);
        match(EmbXParser::T__9);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LiteralContext ------------------------------------------------------------------

EmbXParser::LiteralContext::LiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::LiteralContext::INT() {
  return getToken(EmbXParser::INT, 0);
}

tree::TerminalNode* EmbXParser::LiteralContext::HEX() {
  return getToken(EmbXParser::HEX, 0);
}

tree::TerminalNode* EmbXParser::LiteralContext::FLOAT() {
  return getToken(EmbXParser::FLOAT, 0);
}

tree::TerminalNode* EmbXParser::LiteralContext::STRING() {
  return getToken(EmbXParser::STRING, 0);
}


size_t EmbXParser::LiteralContext::getRuleIndex() const {
  return EmbXParser::RuleLiteral;
}


std::any EmbXParser::LiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitLiteral(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::LiteralContext* EmbXParser::literal() {
  LiteralContext *_localctx = _tracker.createInstance<LiteralContext>(_ctx, getState());
  enterRule(_localctx, 90, EmbXParser::RuleLiteral);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(608);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EmbXParser::T__10) {
      setState(607);
      match(EmbXParser::T__10);
    }
    setState(610);
    _la = _input->LA(1);
    if (!(((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 15) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EndianContext ------------------------------------------------------------------

EmbXParser::EndianContext::EndianContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EmbXParser::EndianContext::LITTLE() {
  return getToken(EmbXParser::LITTLE, 0);
}

tree::TerminalNode* EmbXParser::EndianContext::BIG() {
  return getToken(EmbXParser::BIG, 0);
}

tree::TerminalNode* EmbXParser::EndianContext::NATIVE() {
  return getToken(EmbXParser::NATIVE, 0);
}


size_t EmbXParser::EndianContext::getRuleIndex() const {
  return EmbXParser::RuleEndian;
}


std::any EmbXParser::EndianContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EmbXVisitor*>(visitor))
    return parserVisitor->visitEndian(this);
  else
    return visitor->visitChildren(this);
}

EmbXParser::EndianContext* EmbXParser::endian() {
  EndianContext *_localctx = _tracker.createInstance<EndianContext>(_ctx, getState());
  enterRule(_localctx, 92, EmbXParser::RuleEndian);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(612);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 4035225266123964416) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool EmbXParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 43: return exprSempred(antlrcpp::downCast<ExprContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool EmbXParser::exprSempred(ExprContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 4);
    case 1: return precpred(_ctx, 3);
    case 2: return precpred(_ctx, 2);
    case 3: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

void EmbXParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  embxParserInitialize();
#else
  ::antlr4::internal::call_once(embxParserOnceFlag, embxParserInitialize);
#endif
}
