# EmbX Development Roadmap

The project vision and general design principles are defined in [`VISION.md`](VISION.md). The roadmap implements that vision; it does not redefine the language contract.

The current source state is **EmbX 0.9.49 accepted green through Lesson 11, including generalized terminated sequences**. The 0.9.44 source remains the immutable predecessor reference for reflection/contract closure. 0.9.49 adds the generalized terminated-sequence boundary rule while preserving the canonical AST → Plan → reference-runtime architecture.

## Completed integration history

- add real-format fixtures that can be checked outside the compiler;
- close one-element named-array execution symmetry in the reference codecs;
- harden bit-container execution across the complete 1..64-bit width boundary;
- prefer existing EmbX constructs over format-specific language extensions;
- keep format-specific variable-length/event semantics outside the core until a complete contract exists;
- add a direct regression test through the real compiler and reference decoder;
- keep the fixture small enough for byte-level inspection and audible verification.

The first fixture is Standard MIDI File Type 0. See `docs/MIDI_EXAMPLE.md`.

## Foundation Closure

### F1 — Source and architecture cleanup

- remove confirmed dead helpers and empty translation units;
- remove duplicate build source-list residue;
- remove stale milestone documents from the active source tree;
- synchronize version/status references;
- maintain one authoritative semantic representation per fact;
- keep `Plan` as the executable semantic boundary.

### F2 — Expression semantic closure — CLOSED in 0.9.40

- canonical executable expression representation;
- expression result typing;
- operator type rules;
- checked integer arithmetic;
- boolean short-circuit semantics;
- shared constant/runtime evaluation;
- SymbolId-bound executable identifiers;
- deterministic and negative tests.

The expression-typing closure is complete: after SymbolId binding, general operator validation uses the referenced declaration type. No second expression IR or resolver was introduced.

### F3 — Layout closure — COMPLETE

The active F3 model defines every executable layout shape through `LayoutBounds`: `minSize` plus an optional finite `maxSize`. `Exact` means `minSize == maxSize`, `Bounded` means a finite maximum exists and differs from the minimum, and `Unbounded` means no finite maximum is statically derivable. Checked `uint64_t` arithmetic is mandatory for both bounds. `$next` is implemented in 0.9.28 for `at(...)` offset expressions; runtime and Plan semantics use the canonical built-in identity.


- canonical type-shape normalization;
- static and dynamic layout classification;
- checked offsets, extents, sizes and alignment;
- authoritative layout dependency graph;
- exact/min/max size model;
- nested and variant layout rules.

## Closed language completion sequence

The 0.9.35–0.9.44 milestones closed the language, runtime, generated-C++ and reflection contracts in a deliberate sequence.
The historical milestone details remain in `docs/RELEASE_NOTES.md`; the current implementation inventory is
`docs/LANGUAGE_COMPLETION_MATRIX.md`.

The current matrix intentionally retains a small set of yellow backend/reflection boundaries. Those are qualification
boundaries, not missing semantic mechanisms. Future work must extend the existing Plan contracts rather than introduce
parallel semantic, expression, resolver or layout systems.

## 0.9.49 — generalized terminated sequences — ACCEPTED GREEN

The 0.9.48 terminated-byte implementation is generalized to `Element[*] until ... max ...`. The semantic rule is element-boundary termination, so nested terminated sequences are decoded without raw-byte interference. Existing `bytes until ...` remains compatible.

The implementation reuses the existing AST, Plan, `LayoutBounds`, Reference Runtime, Reflection and generated C++ architecture. Acceptance passed on the clean Windows/MSYS2 UCRT64 build with 73/73 CTest tests.


## Stage 4 — Backend and release hardening

After language semantics stabilize:

- optimized execution paths;
- additional backends;
- generated-code performance work;
- property/fuzz testing;
- conformance corpus expansion;
- release hardening.


## 0.9.45 — executable learning and MIDI conformance — ACCEPTED GREEN

The 0.9.45 stage establishes the learning course as a second practical validation surface for
EmbX. The course is not a separate tutorial project: its examples are executable documentation
and are backed by the same compiler, Plan semantics, reference runtime and generated backends.

### Course introduction

`course/00_INTRODUCTION.md` is the shared entry point for the whole course. It documents the common compiler pipeline, available outputs, CLI operations, encode/decode model, binary-layout reasoning, validation cycle, error categories, and the boundary between common course material and lesson-specific material. Individual lessons should not duplicate this common background.

### Course plan

The course is developed in the following order. Each lesson should introduce the smallest
necessary language concept, contain a runnable `.embx` example, show the corresponding binary
bytes, and add an automated check where the concept is executable. Real-format lessons also
include an external/practical validation where appropriate.

1. **First binary format** — **active**; fields, fixed bytes, integers, encode/decode round-trip.
2. **Integers and byte order** — signed/unsigned widths and explicit endian semantics.
3. **Bit fields** — packed fields, storage width and boundary cases.
4. **Arrays and dynamic dimensions** — fixed, runtime-sized and remaining extents.
5. **Nested structures** — **active**; composition of structures, arrays of structures and type aliases.
6. **Variants and conditionals** — **active**; discriminators, conditional layout and executable selection.
7. **Offsets and alignment** — `at(...)`, `$next`, alignment and layout bounds.
8. **Symbol dependencies** — SymbolId identity, dependency legality and runtime parameters.
9. **Virtual fields and aliases** — semantic projections without duplicating physical layout.
10. **Callbacks and transforms** — operation contracts, direction and transform metadata.
11. **IPv4** — first complete real protocol example using bit fields and network byte order.
12. **General terminated sequences** — structured elements, element-boundary termination, nested terminated sequences and transactional failure.
13. **MIDI** — real SMF conformance, byte corpus, round-trip and audible playback.
14. **Container/packet format** — magic, variants, blocks, alignment and dynamic payloads.
15. **Callback protocol** — a complete practical callback boundary and generated C++ use.
16. **Complete protocol** — a small end-to-end binary format built from the concepts above.

### Per-lesson acceptance contract

Every practical lesson should converge on the same evidence chain:

`lesson → EmbX source → Plan → reference execution → generated backend → binary fixture → test`

For real formats, the final step may additionally be an external validator or observable
result, such as MIDI playback. A lesson is not considered complete merely because its prose
exists: its example must be reproducible from a clean checkout.

### MIDI foundation

- establish `course/` as executable documentation;
- use real binary fixtures alongside every practical lesson;
- add the supplied 20-file MIDI corpus as the first conformance corpus;
- add an independent corpus checker outside the EmbX semantic core;
- make the MIDI demo use conservative SMF events and a deterministic Windows MCI playback helper;
- keep the existing `midi.embx` structural contract minimal;
- use the MIDI fixtures for teaching, regression testing and practical playback;
- keep MIDI-specific event semantics outside the EmbX core until a complete language contract exists.

### Course quality gates

Before a lesson moves from planned to active it must have:

1. a minimal, working EmbX example;
2. an explanation whose terminology matches `docs/LANGUAGE.md`;
3. deterministic test or conformance evidence where applicable;
4. no lesson-specific semantic machinery in the compiler;
5. reproducible commands documented in the lesson;
6. real bytes or a concrete external artifact whenever the lesson teaches binary behavior.

The course therefore becomes a living validation matrix rather than a parallel specification.

### Course progress at 0.9.52 candidate

- Lessons 1–13: accepted and locally validated at **74/74 CTest tests (100%)**.
- Lesson 14 — TLV composition: candidate, adds one CTest registration and uses only existing language mechanisms.
- Lesson 10 — Callbacks and transforms: accepted and locally validated.
- Lesson 11 — IPv4: accepted and locally validated.
- Lesson 13 — MIDI: accepted, with the supplied 20-file corpus, playable Type 0 fixture, and a standard Track Name text metadata event.
- Lesson 15 — Container/packet format: planned.
- Lesson 16 — Callback protocol: planned.
- Lesson 17 — Complete protocol: planned.

The active lessons use only existing accepted language semantics. The course tests are additional executable documentation and do not alter the compiler contract.

The lesson sequence is educational work on top of the accepted language contract; lessons do not introduce special compiler semantics.

### 0.9.45 acceptance — PASSED

The release gate remains the local Windows/MSYS2 UCRT64 build, canonical examples, complete
CTest suite, MIDI corpus checker and manual audible MIDI check. The course itself is expanded
in later 0.9.x stages without changing accepted language semantics merely to satisfy a lesson.

## 0.9.44 — reflection and contract closure — ACCEPTED GREEN

The reflection layer now exposes the remaining contract facts directly from Plan:

- canonical SymbolId identity for reflected fields, virtual fields and aliases;
- alias target SymbolId;
- exact/unbounded member layout classification where the Plan operation already carries the fact;
- struct min/max layout bounds and `Exact`/`Bounded`/`Unbounded` classification;
- transform metadata already present on the canonical physical field;
- existing callback direction and argument metadata.

No second resolver, expression IR, runtime evaluator or layout algorithm was introduced. The local
Windows/MSYS2 UCRT64 gate passed with canonical examples and **61/61 CTest tests (100%)**.

## Permanent gates

Every change must preserve:

- canonical SymbolId identity;
- one NameResolver;
- Plan-only runtime execution;
- deterministic Plan metadata;
- checked 64-bit logical layout;
- full test-suite health;
- clean source and documentation tree.

No compatibility layer for obsolete semantic models is permitted.

## 0.9.43 — generated C++ backend convergence

PASS9–PASS14 close the current generated-C++ backend surface:

- PASS9 — conditional operations;
- PASS10 — variant representation and discriminator-driven codec;
- PASS11 — virtual fields, field aliases and `scale` transforms;
- PASS12 — runtime-parameter propagation and canonical expression emission;
- PASS13 — multidimensional fixed/dynamic arrays;
- PASS14 — generated callback boundary with encode/decode direction filtering and transactional failure handling.

The implementation continues to consume canonical Plan facts. No second resolver, expression IR,
runtime evaluator or layout algorithm is introduced.

### Release gate

Historical 0.9.43 release gate:

1. perform a clean Windows/MSYS2 UCRT64 build;
2. run `run_examples.cmd`;
3. run the complete **61-test** CTest suite;
4. perform the final source/documentation cleanliness check.

Only after that gate should the C++ backend be treated as the stabilized release baseline.
Rust and Python backends remain deferred.
