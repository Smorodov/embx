# Archive manifest — EmbX 0.9.58 Format Reporter accepted green

EmbX 0.9.58 is the accepted Format Reporter baseline. It is based on the accepted 0.9.57 Canonical Source Generator and 0.9.56 GGUF external-adapter closure. The source tree contains the canonical grammar, compiler/runtime implementation, generated-C++ backend, examples, tests, course material and active documentation.

## Archive state

**EmbX 0.9.58 — Format Reporter — ACCEPTED GREEN**

Acceptance gate: clean target-environment build, canonical examples and **82/82 CTest tests (100%)**.

## Included

- `grammar/` — canonical grammar;
- `src/` — compiler, semantic, Plan, runtime, codecs, reflection, reporting and backend implementation;
- `tests/` — current acceptance and contract suite;
- `examples/` — canonical source examples and playable MIDI fixture;
- `course/` — executable learning and external-format conformance material;
- `docs/` — current vision, contracts, status, roadmap, handoff and release notes;
- build and runner scripts;
- `VERSION`.

## Excluded

- build directories;
- generated ANTLR sources;
- binaries and object files;
- generated local protocol logs;
- local test artifacts;
- previous source archives;
- diagnostic/temporary duplicate project snapshots.

## Architectural closure at 0.9.58

- AST → Semantic → IR → Plan remains the canonical semantic path.
- Plan remains the executable semantic boundary.
- Reference Runtime means Encoder/Decoder plus runtime primitives; no third interpreter was introduced.
- `SymbolId` and `NameResolver` remain canonical identity mechanisms.
- logical layout arithmetic remains checked `uint64_t`.
- external format adapters remain outside `src/`.
- Source Generator consumes AST and adds no semantic model.
- Format Reporter consumes Plan/Reflection and the existing LayoutGraph and adds no semantic/layout model.

## Development sequence

0.9.56 — GGUF adapter closure → 0.9.57 — Canonical Source Generator → 0.9.58 — Format Reporter → conformance/property/fuzz corpus → parser/source-generator round-trip → binary conformance → Reference Runtime/generated-backend differential testing → backend hardening → release cleanup.
