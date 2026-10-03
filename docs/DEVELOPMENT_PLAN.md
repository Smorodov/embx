# EmbX Development Plan

## Current baseline

The current audited source baseline is **EmbX 0.9.84**. The preceding 0.9.83 source state was user-verified GREEN in all four build modes with **91/91 registered CTest tests**. The 0.9.84 changes are documentation/version synchronization only. This document is the forward-looking working plan; it complements `DEVELOPMENT_ROADMAP.md` and does not replace the language contracts.

The architectural invariants remain mandatory: AST → semantic analysis → IR → Plan is the canonical path; Plan is the executable semantic boundary; Reference Runtime means the canonical host execution subsystem (`Encoder` + `Decoder` + runtime primitives), not a second interpreter; generated C++/Rust/Python backends independently consume Plan semantics; one SymbolId identity, one NameResolver, one expression semantic model and one layout model are retained. No format-specific compiler mechanism is introduced merely to satisfy a course lesson. A language change requires a demonstrated universal semantic gap; format-specific requirements belong in adapters.

## C++ Generator completion history and qualification boundaries

The four-stage C++ generator completion sequence is now historical planning context. The current 91-test gate and 0.9.84 audit confirm that the generated backend remains within the established Plan boundary. The completion matrix intentionally keeps any narrower qualification areas visible rather than hiding them behind a blanket “complete” label.

## 1. Historical documentation freeze before GGUF

The multidimensional-array contract is now explicit and normative before tensor-format implementation begins. The authoritative document is `MULTIDIMENSIONAL_ARRAY_LAYOUT.md`. The current EmbX rule is one contiguous order: dimension 0 is outermost and the last dimension varies fastest.

The following must not be added as part of Lesson 15 merely to accommodate GGUF:

- `row_major` / `column_major` syntax;
- arbitrary `stride`;
- tensor-specific AST or Plan types;
- GGUF-specific runtime or layout evaluators.

GGUF/GGML dimension conventions must instead be mapped explicitly to the existing EmbX order and proven with byte-level fixtures.

For runtime/backend APIs, multidimensional decoded data may be exposed as a small non-owning array view: data reference, total element count, element size, dimension count and resolved dimensions. This is target-specific representation, not a new semantic type. See `MULTIDIMENSIONAL_ARRAY_VIEW.md`.

## 2. Historical Lesson 15 — GGUF external adapter capstone

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

## 7.1 — Binary conformance corpus

The binary conformance corpus fixes observable behavior at the executable Plan and Reference Runtime boundary. Valid fixtures must decode with complete input consumption and re-encode to exactly the original bytes. Invalid fixtures must be rejected. The corpus reuses existing lesson/example binary artifacts and does not introduce format-specific language constructs or a second semantic model.

Initial coverage includes the first binary format, IPv4, terminated sequences, TLV, MIDI, and an invalid TLV length fixture.

## 7.2 — Source corpus round-trip foundation

The first post-0.9.58 quality stage is the parser/source-generator corpus gate. A dedicated CTest target discovers every `.embx` source under `examples/` and `course/` and verifies:

`source → AST → canonical source → AST → canonical source`

The two generated canonical sources must be identical. The corpus is discovered from the repository tree rather than maintained as a second hand-written list, so adding a new example or lesson automatically extends the gate. This stage adds no language semantics and no second parser or AST model.

## 7. Conformance foundation and differential qualification

The Source Generator and Format Reporter are closed. The 0.9.59–0.9.60 conformance foundation is also closed: source round-trip, binary conformance, generated-backend differential checks, and the initial differential matrix are all green. The next quality phase should expand coverage without changing language semantics. Priority order:

1. differential matrix expansion using existing course features;
2. property/fuzz testing around parser, Plan construction and binary boundaries;
3. generated-backend hardening and parity checks;
4. Rust/Python backends when justified by a concrete use case;
5. final release/documentation cleanup before EmbX 1.0 planning.

The goal is maturity and closure, not accumulation of mechanisms.

## 7.3 — 0.9.61 differential layout expansion — ACCEPTED GREEN

The 0.9.61 differential stage qualifies existing layout semantics in the generated C++ backend against the canonical host execution path. Coverage includes runtime-sized arrays, dynamic `at(offset)`, `align(4)` inside the offset region, preservation of the caller sequential cursor after `at`, and a zero-length dynamic-array boundary. Acceptance is **87/87 CTest tests (100%)**.

The corpus intentionally uses an identifier-based offset expression rather than `$next + N`: the generated C++ backend currently supports literal and field-identifier layout expressions, while `$next` arithmetic is already covered by the Reference Runtime layout tests. This is a backend qualification boundary, not a reason to alter the language or Plan model.

The 0.9.62 terminated-sequence, 0.9.63 runtime-parameter and 0.9.64 access qualifications are closed. The next quality increments are remaining differential qualification of existing features, followed by property/fuzz testing.

### 7.4 — 0.9.62 terminated-sequence differential qualification — ACCEPTED GREEN

The generated C++ backend is qualified against the existing generalized terminated-sequence contract. Coverage includes nested terminated structured elements, element-boundary outer termination and transactional failure when the outer terminator is absent. Acceptance is **88/88 CTest tests (100%)**.

### 7.6 — 0.9.64 differential access qualification — ACCEPTED GREEN

The generated C++ backend is qualified against the canonical host execution path for existing virtual fields, field aliases and scalar `scale` transforms. The corpus verifies exact encode bytes, generated decode agreement, alias write-through behavior and logical transform round-trip. The encode path now evaluates virtual fields in its working copy before later operations consume them, matching Plan execution order. No language, AST, IR or Plan semantic change was introduced. Acceptance is **90/90 CTest tests (100%)**.

### 7.5 — 0.9.63 runtime-parameter differential qualification — ACCEPTED GREEN

The generated C++ backend is qualified against the existing typed runtime-parameter contract. Coverage includes independent caller-supplied parameter values, a parameter-driven dynamic array dimension and an identifier-based `at(delta)` offset. Encode and decode are compared against the canonical host execution path. Acceptance is **89/89 CTest tests (100%)**. No language, AST, IR or Plan semantic change was introduced.

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

1. Preserve EmbX 0.9.84 as the audited documentation/source baseline.
2. Keep the existing 91-test full gate and four build profiles unchanged unless a concrete deficiency is demonstrated.
3. For the next substantive change, add the smallest contract and regression test first, then qualify it against the Reference Runtime and generated C++ where applicable.
4. Do not introduce parallel semantic, expression, resolver, layout or format-specific mechanisms.

### 0.9.61 Differential layout expansion — ACCEPTED GREEN

The 0.9.61 expansion keeps the established generated-backend differential architecture and adds one dedicated corpus for existing dynamic-dimension and explicit-layout semantics: runtime-sized arrays, identifier-based `at(offset)`, and `align(4)` within an offset region. Encode and decode are compared against the canonical host execution path, including a zero-length dynamic-array boundary. No AST, IR, Plan or language construct is introduced. The stage is accepted at 87/87 tests. `$next` arithmetic remains covered by the existing Reference Runtime layout tests and is a separate generated-backend qualification boundary.

### 0.9.60 Differential conformance matrix — ACCEPTED GREEN

The generated C++ backend is qualified incrementally against the canonical host execution path. The accepted 0.9.60 matrix covers explicit byte order, bit fields, fixed arrays, nested structures, and variants in both directions. The matrix uses one dedicated corpus and one generated translation unit so harness mechanics do not obscure semantic differential failures. The same Plan semantics and representative values are used on both sides; no second oracle or language extension is introduced. Acceptance is **86/86 CTest tests (100%)**.

### 0.9.63 — runtime-parameter differential qualification — ACCEPTED GREEN

The generated C++ backend is qualified for typed runtime parameters using the existing Plan parameter metadata. The corpus checks independent caller-supplied values for a dynamic array dimension and an identifier-based `at(delta)` offset, with encode/decode compared against the canonical host execution path. Acceptance is **89/89 CTest tests (100%)**. No language, AST, IR or Plan semantic change was introduced. The conformance test resolves parameter symbols by canonical qualified identity.

### 0.9.62 — terminated-sequence differential qualification — ACCEPTED GREEN

The next differential slice qualifies the already implemented generalized terminated-sequence
semantics against generated C++. The corpus reuses Lesson 12 and covers nested terminated
structured elements, exact wire equality for encode/decode, and transactional failure when the
outer terminator is absent. No new language mechanism is planned.
