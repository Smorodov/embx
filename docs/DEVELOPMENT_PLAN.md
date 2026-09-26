# EmbX Development Plan

## Current baseline

The accepted reference point is EmbX 0.9.58 at 82/82 CTest tests; 0.9.57 is the accepted Canonical Source Generator and 0.9.56 is the accepted Lesson 15.3 GGUF external-adapter closure. The universal array-buffer boundary remains frozen. This document is the forward-looking working plan. It complements `DEVELOPMENT_ROADMAP.md`; it does not replace the language contracts.

The architectural invariants remain mandatory: AST → semantic analysis → Plan is the canonical path; Plan is the executable semantic boundary; Reference Runtime means the canonical host execution subsystem (`Encoder` + `Decoder` + runtime primitives), not a second interpreter; generated C++/Rust/Python backends independently consume Plan semantics; one SymbolId identity, one NameResolver, one expression semantic model and one layout model are retained. No format-specific compiler mechanism is introduced merely to satisfy a course lesson. A language change requires a demonstrated universal semantic gap; format-specific requirements belong in adapters.

## 1. Documentation freeze before GGUF

The multidimensional-array contract is now explicit and normative before tensor-format implementation begins. The authoritative document is `MULTIDIMENSIONAL_ARRAY_LAYOUT.md`. The current EmbX rule is one contiguous order: dimension 0 is outermost and the last dimension varies fastest.

The following must not be added as part of Lesson 15 merely to accommodate GGUF:

- `row_major` / `column_major` syntax;
- arbitrary `stride`;
- tensor-specific AST or Plan types;
- GGUF-specific runtime or layout evaluators.

GGUF/GGML dimension conventions must instead be mapped explicitly to the existing EmbX order and proven with byte-level fixtures.

For runtime/backend APIs, multidimensional decoded data may be exposed as a small non-owning array view: data reference, total element count, element size, dimension count and resolved dimensions. This is target-specific representation, not a new semantic type. See `MULTIDIMENSIONAL_ARRAY_VIEW.md`.

## 2. Lesson 15 — GGUF external adapter capstone

GGUF is the next practical real-format target, but its parser and format semantics are strictly outside the EmbX core. Keep the supplied original GGUF specification unchanged as the external reference in `docs/GGUF_SPECIFICATION.md`. Keep `course/15_gguf/GGUF_IN_EMBX.md` as the course-oriented specification expressed in EmbX terminology.

Stages:
1. Minimal valid GGUF structural parser.
2. Length-prefixed UTF-8 strings and metadata scalar values.
3. Tagged metadata values and arrays, including nested-array investigation.
4. Tensor information and dimension arrays.
5. Alignment, tensor-data region and relative offsets.
6. Explicit GGML↔EmbX dimension-order mapping proven with non-square byte-level fixtures.
7. Independent real GGUF fixtures generated with the external Python `gguf` library available in the user's conda environment.
8. Reference Runtime and generated-backend decode/conformance checks.
9. Round-trip and byte-level checks where the representation is deterministic.
10. Negative fixtures for truncation, invalid magic and other concrete format failures.
11. Deep audit before moving to another major feature.

Before tensor conformance is considered closed, complete the multidimensional exact-byte matrix (2×3, 3×2, 2×3×4 and dynamic/nested cases) and verify that any future array-view API preserves the same shape and element order.

The external `gguf` package is an oracle/fixture producer or validator only. The GGUF adapter itself is also not an EmbX semantic dependency. It must not become an EmbX semantic dependency. If GGUF exposes a construct not expressible by current EmbX, first demonstrate the smallest concrete language gap and determine whether existing constructs can express it. Only then consider a language change.

## 3. Post-GGUF deep audit — completed

The project-wide semantic/conformance audit was completed before the Source Generator and Format Reporter stages. The audited invariants remain: one authoritative representation per semantic fact, AST → Semantic → IR → Plan as the semantic path, canonical host execution through Encoder/Decoder + runtime primitives, checked 64-bit logical layout arithmetic, external adapters outside `src/`, and synchronized documentation.

## 3.1 Historical audit gates

The following gates were the pre-0.9.57 audit checklist and are retained as historical acceptance criteria. They are now closed. The audit must verify one authoritative representation for each semantic fact, the Format Independence boundary, absence of obsolete compatibility paths, strict warning cleanliness, complete documentation synchronization, and clean-build/test/example gates. No new language mechanism is introduced by this phase.

### Audit gates

- source inventory and CMake ownership are complete;
- AST → Semantic → IR → Plan remains the only semantic path;
- Reference Runtime remains the canonical host execution reference (`Encoder`/`Decoder` + runtime primitives);
- Encoder/Decoder symmetry is preserved without duplicated semantic models;
- logical layout arithmetic remains checked `uint64_t`;
- external adapters remain outside `src/`;
- all active documentation agrees with the accepted version;
- clean build, canonical examples and full CTest suite remain green.

## 4. EmbX Source Generator — accepted green

The Canonical Source Generator stage is closed at 81/81 tests. Its normative contract and AST coverage document remain active.

## 4.1 Historical contract-first stage

The detailed normative contract is `SOURCE_GENERATOR_CONTRACT.md`. Implementation must begin only after the contract and coverage inventory are frozen.

## 4.2 Source Generator implementation record

Add a generator that converts the canonical AST into deterministic, canonical, human-readable EmbX source. This is a source reconstruction/pretty-printing facility, not an additional execution backend.

Architecture:

`EmbX source → Parser → AST → Source Generator → canonical EmbX source`

The generator must use the existing AST and must not introduce its own semantic resolver, expression IR, evaluator or layout algorithm. It must report an error when an AST construct has no canonical EmbX representation rather than silently inventing syntax.

### Source-generator stages

- canonical formatting and indentation;
- deterministic declaration and field rendering;
- types, dimensions and expressions;
- variants and conditionals;
- aliases and virtual fields;
- offsets, alignment and layout constructs;
- callbacks and transforms;
- documentation/comments and supported attributes;
- complete current AST coverage.

### Mandatory conformance tests

For a source file `S`:

`AST1 = parse(S)`

`S2 = generate(AST1)`

`AST2 = parse(S2)`

Require semantic/structural AST equivalence. Then test idempotence:

`generate(parse(generate(parse(S)))) == generate(parse(S))`

The corpus should include all current examples and course sources, especially terminated sequences, TLV and GGUF.

## 5. Format Reporter — accepted green

The Format Reporter stage is closed at 82/82 tests. It consumes Plan/Reflection and the existing LayoutGraph only; it does not calculate independent layout semantics.

Add a human-readable format summary generated from the canonical AST/Plan. It is deliberately separate from source reconstruction, although both consume the same canonical compiler data.

The reporter should expose facts already established by Plan and Reflection rather than recomputing binary layout. No second size/layout engine is permitted.

### Required summary levels

**Module summary**
- module name;
- structures, variants and other relevant declarations;
- overall minimum/maximum frame size where the Plan can establish them;
- exact/bounded/unbounded/unknown layout counts;
- alignment requirements;
- dynamic fields;
- runtime parameters;
- aliases/virtual fields;
- dependency counts;
- byte-order usage.

**Type summary**
- type name;
- layout class;
- minimum size;
- maximum size when finite;
- fixed/exact status;
- alignment;
- field count.

**Field details**
- type and dimensions;
- physical offset when known;
- size/bounds when known;
- dependencies;
- conditions/variant membership;
- alias/virtual relationship;
- relevant byte order.

The exact report syntax is intentionally deferred until the existing Plan/Reflection facts are audited for completeness.

## 6. Round-trip and documentation quality gate

The Source Generator and Format Reporter should become part of the quality infrastructure rather than lesson-specific features. The desired closed loop is:

`source → AST → Plan → canonical source + format report`

The generated source must parse back to an equivalent AST. The report must agree with Plan/Reflection facts. This creates an independent way to detect drift between parser, AST, Plan and documentation.

## 7. After these two tools

Once GGUF, Source Generator and Format Reporter are stable, stop adding lessons merely for feature count and perform a deep semantic/conformance audit. Priority order:

1. corpus and property/fuzz testing;
2. parser/source-generator round-trip corpus;
3. binary conformance corpus;
4. Reference Runtime versus generated backend differential tests;
5. backend hardening;
6. Rust/Python backends when justified;
7. release/documentation cleanup.

The goal is maturity and closure, not accumulation of mechanisms.

## 8. Decision rules for future work

Before adding a language feature:
1. identify the concrete external-format or user requirement;
2. show why existing EmbX cannot express it;
3. identify the smallest missing semantic concept;
4. verify that the concept belongs in the canonical AST/Plan model;
5. add tests and documentation before backend-specific work;
6. update all affected contracts and the course only after the semantic rule is closed.

Before adding a new analysis/reporting mechanism:
- prefer existing Plan/Reflection facts;
- do not duplicate layout, dependency or expression evaluation;
- keep output deterministic;
- add round-trip/differential tests where applicable.

## Immediate next actions

1. Freeze the audited 0.9.58 source archive.
2. Build the parser/source-generator round-trip corpus across examples and course sources.
3. Add property/fuzz coverage around parser, Plan construction and binary boundaries.
4. Add binary conformance corpus checks and Reference Runtime versus generated-backend differential tests.
5. Re-audit backend qualification before considering Rust/Python targets.
6. Perform release/documentation cleanup before EmbX 1.0 planning.

