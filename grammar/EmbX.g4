grammar EmbX;

module: DOC_COMMENT? namespaceDecl? importDecl* item* EOF;
namespaceDecl: NAMESPACE qualifiedName ';';
importDecl: IMPORT STRING (AS ID)? ';';
qualifiedName: ID ('::' ID)*;
item: docComment? (attributeDecl | endianDirective | constDecl | computedDecl | enumDecl | structDecl | callbackDecl | parameterDecl | typeAlias);

endianDirective: AT ENDIAN endian;
constDecl: attributes? CONST ID '=' expr ';';
computedDecl: attributes? COMPUTED ID '=' expr ';';
enumDecl: attributes? ENUM ID (':' typeRef)? '{' enumItem (',' enumItem)* ','? '}';
enumItem: docComment? ID '=' expr;
typeAlias: attributes? TYPE ID '=' typeRef ';';

structDecl: attributes? STRUCT ID (endian)? '{' (requiresDecl | member)* '}';
requiresDecl: REQUIRES expr ';';
member: docComment? (fieldDecl | virtualDecl | aliasDecl | bitsBlock | variantDecl | conditionalDecl | blockDecl | atDecl | alignDecl | callbackUse);

virtualDecl: attributes? LET ID '=' expr ';';
aliasDecl: attributes? ALIAS ID '=' ID ';';

fieldDecl: attributes? ID ':' typeRef fieldModifier* transform? ( '=' literal )? ';';
fieldModifier: endian | '[' expr ']' | '(' INT ')' ;
transform: TRANSFORM ID '(' expr (',' expr)* ')' ;

bitsBlock: attributes? BITS '{' bitField* '}';
bitField: docComment? ID ':' typeRef '(' INT ')' fieldModifier* ( '=' literal )? ';';

conditionalDecl: attributes? IF '(' expr ')' '{' member* '}' (ELSE '{' member* '}')?;
variantDecl: attributes? VARIANT ID BY expr '{' variantCase+ variantDefault? '}';
variantCase: expr ':' variantBody;
variantDefault: DEFAULT ':' variantBody;
variantBody: typeRef ';' | '{' member* '}';

blockDecl: attributes? BLOCK ID? '[' expr ']' '{' member* '}';
atDecl: attributes? ATCALL '(' expr ')' '{' member* '}';
alignDecl: attributes? ALIGN '(' expr ')' ';';

callbackDecl: attributes? CALLBACK ID ('(' ID? ')')? ';';
parameterDecl: attributes? PARAM ID ':' typeRef ';';
callbackUse: attributes? CALLBACK ID '(' argList? ')' ';';
argList: expr (',' expr)*;

docComment: DOC_COMMENT+;

attributeDecl: ATTRIBUTE ID (':' attributeType)? ';';
attributeType: U8|I8|U16|I16|U32|I32|U64|I64|F32|F64|STRINGTYPE;
attributes: '[' attributeEntry (',' attributeEntry)* ','? ']';
attributeEntry: ID ('=' (literal | ID | TRUE | FALSE))?;

typeRef: (qualifiedName | baseType) typeSuffix? terminatedSequenceSuffix? ;
terminatedSequenceSuffix: UNTIL HEX+ MAX INT ;
typeSuffix: '[' (STAR | expr) ']' ;
baseType: primitive | ID | BYTES | STRINGTYPE;
primitive: U8|I8|U16|I16|U32|I32|U64|I64|F32|F64;

expr: primary
    | '-' expr
    | expr op=('*'|'/'|'%') expr
    | expr op=('+'|'-') expr
    | expr op=('=='|'!='|'<'|'<='|'>'|'>=') expr
    | expr op=('&&'|'||') expr
    ;
primary: INT | HEX | FLOAT | STRING | TRUE | FALSE | NEXT | SIZE_IN_BYTES | MIN_SIZE_IN_BYTES | MAX_SIZE_IN_BYTES | qualifiedName | '(' expr ')';
literal: '-'? (INT | HEX | FLOAT | STRING);

endian: LITTLE | BIG | NATIVE;

STAR:'*'; UNTIL:'until'; MAX:'max';
SCOPE:'::';
AT: '@'; ATTRIBUTE:'attribute'; REQUIRES:'requires'; ENDIAN:'endian'; CONST:'const'; COMPUTED:'computed'; ENUM:'enum'; STRUCT:'struct'; TYPE:'type'; BITS:'bits'; VARIANT:'variant'; BY:'by'; DEFAULT:'default'; BLOCK:'block'; ATCALL:'at'; ALIGN:'align'; CALLBACK:'callback'; PARAM:'param'; IF:'if'; ELSE:'else'; LET:'let'; NAMESPACE:'namespace'; IMPORT:'import'; AS:'as'; ALIAS:'alias'; TRANSFORM:'transform'; NEXT:'$next'; TRUE:'true'; FALSE:'false'; SIZE_IN_BYTES:'$size_in_bytes'; MIN_SIZE_IN_BYTES:'$min_size_in_bytes'; MAX_SIZE_IN_BYTES:'$max_size_in_bytes';
LITTLE:'little'; BIG:'big'; NATIVE:'native';
U8:'u8'; I8:'i8'; U16:'u16'; I16:'i16'; U32:'u32'; I32:'i32'; U64:'u64'; I64:'i64'; F32:'f32'; F64:'f64'; BYTES:'bytes'; STRINGTYPE:'string';
ID:[a-zA-Z_][a-zA-Z0-9_]*;
HEX:'0x'[0-9a-fA-F]+;
FLOAT:[0-9]+'.'[0-9]+;
INT:[0-9]+;
STRING:'"' (~["\\] | '\\' .)* '"';
WS:[ \t\r\n]+ -> skip;
DOC_COMMENT: '///' ~[\r\n]* | '/**' .*? '*/';
LINECOMMENT:'//' ~[\r\n]* -> skip;
BLOCKCOMMENT:'/*' .*? '*/' -> skip;