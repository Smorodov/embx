# EmbX Source Generator Contract

Status: design contract / 0.9.57

## 1. Purpose

The Source Generator reconstructs canonical, human-readable EmbX source from the canonical AST.

It is a **source reconstruction tool**, not an execution backend, semantic analyzer, formatter for arbitrary source text, or second compiler representation.

Canonical pipeline:

`EmbX source → Parser → AST → Source Generator → canonical EmbX source`

The generator must preserve the semantics represented by the AST while choosing one deterministic source spelling for every supported construct.

## 2. Ownership

The Source Generator owns only source rendering:

- indentation;
- whitespace;
- declaration ordering as represented by the AST;
- canonical punctuation and delimiters;
- deterministic expression/source spelling;
- rendering of supported documentation and attributes.

It must not own:

- name resolution;
- SymbolId assignment;
- semantic validation;
- type inference;
- dependency analysis;
- layout calculation;
- expression evaluation;
- Plan construction.

No second semantic or layout model is permitted.

## 3. Input boundary

The generator consumes the existing AST only.

It must not require Plan, IR, parser contexts, token streams, or source text to reconstruct ordinary supported syntax.

If a construct is intentionally represented only after semantic lowering and has no AST-level source representation, it is not silently reconstructed from Plan facts. Such a case is an explicit coverage gap and must be diagnosed.

## 4. Determinism

For one equivalent AST, generation must produce byte-for-byte identical output.

Output must not depend on:

- pointer addresses;
- hash-table iteration order;
- process state;
- locale;
- timestamps;
- filesystem order;
- compiler/runtime randomness.

Where declarations are stored in ordered AST containers, that source order is authoritative. Where the AST represents an unordered collection, the generator must define and document one deterministic ordering before implementation.

## 5. Canonical formatting

The first implementation uses one fixed formatting policy:

- four spaces per indentation level;
- one declaration per logical line where the grammar permits;
- stable brace placement;
- no trailing whitespace;
- exactly one final newline;
- stable blank-line policy between top-level declaration groups;
- canonical spelling of literals, types, dimensions and operators.

The generator must never perform semantic simplification merely to improve formatting.

## 6. Required AST coverage

The initial implementation must inventory and cover every currently constructible AST node, including at minimum:

- module and documentation;
- structs;
- fields and field documentation;
- scalar and named types;
- dimensions, including dynamic/remaining dimensions;
- expressions and parentheses where semantically significant;
- aliases;
- constants;
- enums and enum items;
- variants;
- conditionals;
- `at` and `align` operations;
- blocks;
- callbacks;
- transforms;
- virtual fields and aliases;
- supported attributes/requires;
- runtime parameters;
- byte-order declarations/defaults where present in the AST.

The inventory itself becomes a test/maintenance checklist.

## 7. Unsupported constructs

If the AST contains a construct without a canonical EmbX spelling, generation must fail explicitly with a diagnostic identifying the construct.

The generator must not:

- emit invented extension syntax;
- drop the construct;
- replace it with an approximation;
- silently serialize implementation-specific Plan details.

## 8. Round-trip contract

For every corpus source `S`:

`AST1 = parse(S)`

`S2 = generate(AST1)`

`AST2 = parse(S2)`

`AST2` must be structurally and semantically equivalent to `AST1` under the existing compiler contracts.

The generator is not required to preserve original whitespace, comments that have no AST representation, or non-canonical lexical spelling.

## 9. Idempotence

Canonical generation must be idempotent:

`generate(parse(generate(parse(S)))) == generate(parse(S))`

The comparison is byte-for-byte after generation.

## 10. Corpus

The conformance corpus must include:

- all current `examples/*.embx` sources;
- all course sources;
- terminated sequences;
- TLV;
- runtime parameters;
- variants and conditionals;
- nested/multidimensional arrays;
- callbacks/transforms;
- aliases/virtual fields;
- GGUF course source where it is an ordinary EmbX source rather than adapter implementation code.

External adapter implementation files are not generator input merely because they are part of the repository.

## 11. Error contract

Errors must be deterministic and must identify the unsupported AST construct and, where available, its source location/name.

A failed generation must not return partial output as a successful canonical source.

## 12. Non-goals

The Source Generator does not:

- regenerate source from Plan;
- normalize or optimize the language semantics;
- introduce new language constructs;
- generate C++/Rust/Python;
- replace the parser;
- replace documentation tooling;
- become a format adapter.

## 13. Acceptance gate

The stage is closed only when:

1. the AST coverage inventory is complete;
2. all supported corpus sources round-trip;
3. canonical generation is deterministic;
4. idempotence passes;
5. unsupported AST constructs fail explicitly;
6. no semantic/layout mechanism was duplicated;
7. existing full CTest and example gates remain green.

Only after this gate should the Format Reporter be implemented.
