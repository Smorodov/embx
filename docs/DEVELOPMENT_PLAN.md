# EmbX Development Plan

## Current baseline

The current accepted reference point is EmbX 0.9.52: Lessons 1–14 are accepted and locally validated at 75/75 CTest tests. This document is the forward-looking working plan. It complements `DEVELOPMENT_ROADMAP.md`; it does not replace the language contracts.

The architectural invariants remain mandatory: AST → semantic analysis → Plan is the canonical path; Plan is the executable semantic boundary; Reference Runtime is the semantic reference implementation; generated C++/Rust/Python backends independently consume Plan semantics; one SymbolId identity, one NameResolver, one expression semantic model and one layout model are retained. No format-specific compiler mechanism is introduced merely to satisfy a course lesson.

## 1. Lesson 15 — GGUF capstone

GGUF is the next practical real-format target. Keep the supplied original GGUF specification unchanged as the external reference in `docs/GGUF_SPECIFICATION.md`. Keep `course/15_gguf/GGUF_IN_EMBX.md` as the course-oriented specification expressed in EmbX terminology.

Stages:
1. Minimal valid GGUF header.
2. Length-prefixed UTF-8 strings and metadata scalar values.
3. Tagged metadata values and arrays, including nested-array investigation.
4. Tensor information and dimension arrays.
5. Alignment, tensor-data region and relative offsets.
6. Real GGUF fixtures generated with the external Python `gguf` library available in the user's conda environment.
7. Reference Runtime and generated-backend decode/conformance checks.
8. Round-trip and byte-level checks where the representation is deterministic.
9. Negative fixtures for truncation, invalid magic and other concrete format failures.
10. Deep audit before moving to another major feature.

The external `gguf` package is an oracle/fixture producer or validator only. It must not become an EmbX semantic dependency. If GGUF exposes a construct not expressible by current EmbX, first demonstrate the smallest concrete language gap and determine whether existing constructs can express it. Only then consider a language change.

## 2. EmbX Source Generator — planned after the GGUF capstone

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

## 3. Format Reporter — planned together with the Source Generator

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

## 4. Round-trip and documentation quality gate

The Source Generator and Format Reporter should become part of the quality infrastructure rather than lesson-specific features. The desired closed loop is:

`source → AST → Plan → canonical source + format report`

The generated source must parse back to an equivalent AST. The report must agree with Plan/Reflection facts. This creates an independent way to detect drift between parser, AST, Plan and documentation.

## 5. After these two tools

Once GGUF, Source Generator and Format Reporter are stable, stop adding lessons merely for feature count and perform a deep semantic/conformance audit. Priority order:

1. corpus and property/fuzz testing;
2. parser/source-generator round-trip corpus;
3. binary conformance corpus;
4. Reference Runtime versus generated backend differential tests;
5. backend hardening;
6. Rust/Python backends when justified;
7. release/documentation cleanup.

The goal is maturity and closure, not accumulation of mechanisms.

## 6. Decision rules for future work

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

1. Begin GGUF Stage 15.1 with the smallest valid header.
2. Produce at least one golden GGUF fixture using the external Python `gguf` package.
3. Express that fixture using existing EmbX only.
4. Validate Plan and Reference Runtime behavior.
5. Continue through the GGUF stages above before implementing the Source Generator.
6. After GGUF closure, implement the canonical Source Generator and its round-trip tests.
7. Implement the Format Reporter using existing Plan/Reflection facts.
