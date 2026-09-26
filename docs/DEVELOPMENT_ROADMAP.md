# EmbX Development Roadmap

The project vision and general design principles are defined in [`VISION.md`](VISION.md). The roadmap implements that vision; it does not redefine the language contract.

The accepted reference is **EmbX 0.9.58 green**. 0.9.56 closes the GGUF external-adapter stage, 0.9.57 closes the Canonical Source Generator, and 0.9.58 closes the Format Reporter. The next working phase is semantic/conformance hardening and quality infrastructure. The 0.9.44 source remains the immutable predecessor reference for reflection/contract closure. 0.9.49 adds the generalized terminated-sequence boundary rule while preserving the canonical AST → Plan → reference-runtime architecture.

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


## 0.9.53 — universal array buffer boundary — ACCEPTED GREEN

The multidimensional-array model now has a concrete runtime boundary: `ArrayDescriptor + ArrayBuffer`, composed as a non-owning `ArrayView`. The first implementation is intentionally representation-only. It does not replace `value::Value::Array`, alter Plan semantics, or introduce tensor/matrix/stride concepts. Implementation-02 connected `Plan::Type` to `ArrayDescriptor` through `plan::makeArrayDescriptor()`, deriving element metadata and checked shape from the canonical Plan. Implementation-03 exercises the descriptor against the actual encoded buffer of a `Named[2][3]` array and verifies the canonical traversal order with distinct bytes. Implementation-04 adds checked `ArrayView` indexed access plus dynamic-dimension and nested-structure coverage, still reusing the existing evaluator and Plan layout machinery.

The multidimensional array closure is complete: descriptor/buffer/view separation, Plan integration, dynamic dimensions, nested structures and conformance coverage were accepted at 79/79. The GGUF adapter is now accepted separately at 80/80.

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
14. **TLV composition** — accepted; demonstrates dynamic byte payloads, structured remaining sequences and terminated boundaries using existing language mechanisms.
15. **GGUF capstone** — planned next; real-world external-format conformance with explicit GGML↔EmbX tensor-dimension mapping.

Earlier container/packet, callback and complete-protocol material remains part of the historical course corpus and accepted implementation evidence; it is not a competing future lesson-number sequence.

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

### Course progress at 0.9.58 accepted green

- Lessons 1–15 remain accepted; the 0.9.56 GGUF adapter closure, 0.9.57 Source Generator and 0.9.58 Format Reporter bring the current full suite to **82/82 CTest tests (100%)** without changing the EmbX language semantics.
- Lesson 14 — TLV composition: accepted, adds one CTest registration and uses only existing language mechanisms.
- Lesson 10 — Callbacks and transforms: accepted and locally validated.
- Lesson 11 — IPv4: accepted and locally validated.
- Lesson 13 — MIDI: accepted, with the supplied 20-file corpus, playable Type 0 fixture, and a standard Track Name text metadata event.
- Lesson 15 — GGUF capstone: accepted through stage 15.3 as an external adapter; no GGUF semantics enter the language core.

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


## Multidimensional array contract — CLOSED for the current language

The previous documentation gap around multidimensional physical order is closed before GGUF work. `MULTIDIMENSIONAL_ARRAY_LAYOUT.md` is normative. EmbX uses one contiguous traversal: dimension 0 is outermost and the last dimension varies fastest. Shape, element order, byte order, alignment and external tensor strides are documented as separate concepts.

The implementation must not grow `row_major`, `column_major` or arbitrary `stride` syntax solely for GGUF. GGUF/GGML dimension conventions are an explicit boundary-mapping problem and require byte-level conformance fixtures.

## Planned tooling — canonical source generation and format reporting

After the GGUF capstone, add two related developer tools without introducing new semantic machinery.

### EmbX Source Generator

Generate deterministic, canonical, human-readable `.embx` source from the canonical AST. The generator is a source reconstruction/pretty-printer, not an execution backend. It must reuse the existing AST and must not introduce a second resolver, expression IR, evaluator or layout algorithm.

Mandatory acceptance: `parse → generate → parse` produces an equivalent AST, and `generate → parse → generate` is idempotent. The corpus must include the existing examples and course sources.

### EmbX Format Reporter

Generate a human-readable summary from existing Plan/Reflection facts. At minimum report minimum/maximum frame or type size where known, layout class, fixed/exact status, alignment, field counts, dynamic fields, dependencies, runtime parameters, aliases/virtual fields and byte-order usage. The reporter must not recalculate layout independently.

These tools form a closed documentation/conformance loop:

`source → AST → Plan → canonical source + format summary`

The detailed implementation plan is maintained in `docs/DEVELOPMENT_PLAN.md`.

## 0.9.58 — Format Reporter — ACCEPTED GREEN

## 0.9.59 — Source corpus round-trip foundation — IN PROGRESS

The first post-0.9.58 quality stage promotes parser/source-generator round-trip checking to a repository-wide corpus gate. The test discovers all `.embx` files under `examples/` and `course/`, parses each source, generates canonical source, parses the generated source again, and requires the second canonical generation to be byte-identical to the first. No language semantics are added.

The Format Reporter is a deterministic presentation layer over canonical Plan/Reflection facts. Dependency counts use the existing LayoutGraph; facts not established by the canonical model are reported as `unknown` rather than inferred. No second semantic or layout model was introduced.

The clean target-environment validation passed at **82/82 CTest tests (100%)**.

## 0.9.57 — Canonical Source Generator — ACCEPTED GREEN

The Source Generator reconstructs deterministic canonical EmbX source from the AST. It does not introduce a second resolver, expression model, evaluator or layout engine. Structural AST validation rejects incomplete or unsupported constructs instead of emitting partial source.

The clean target-environment validation passed at **81/81 CTest tests (100%)**.

## 0.9.56 — GGUF external adapter stage 15.3 — ACCEPTED GREEN

Stage 15.3 closes the adapter test boundary without changing EmbX core. Coverage now includes non-square 3-D dimension mapping, multiple aligned tensors, nested metadata arrays, unsupported versions, tensor-size overflow and tensor-data bounds. This historical stage was followed by the accepted 0.9.57 Source Generator and 0.9.58 Format Reporter stages.

The accepted result is that GGUF remains entirely external: no GGUF-specific AST, Plan, runtime type, layout evaluator or array semantic is introduced. The full suite is validated at 80/80 CTest tests (100%).

## 0.9.54 — GGUF external adapter stage 15.1 — ACCEPTED GREEN

The first GGUF stage is deliberately outside EmbX core. It adds a standalone structural parser/adapter for GGUF v3 and maps only fixed-size, unquantized tensor representations to the existing `ArrayDescriptor`/`ArrayBuffer` boundary. No GGUF-specific AST, Plan, runtime type or layout mechanism is introduced.

The new adapter is the only layer that knows GGUF metadata tags, tensor type codes, alignment, offsets and external dimension conventions. Quantized representations remain adapter-owned.

## Next capstone — Lesson 15: GGUF

Use GGUF as a real-world conformance target without introducing GGUF-specific compiler semantics. Preserve the supplied original GGUF specification as `docs/GGUF_SPECIFICATION.md` and maintain `course/15_gguf/GGUF_IN_EMBX.md` as the course-oriented restatement.

The sequence is: (1) consume the frozen multidimensional-array contract, (2) smallest valid GGUF header, (3) strings and metadata scalars, (4) tagged metadata arrays including nested-array investigation, (5) tensor info and explicit GGML↔EmbX dimension mapping, (6) alignment/tensor-data region, (7) independent fixtures, (8) Reference Runtime and generated-backend conformance, and (9) deep audit. Stage 15.2 closes the mapping/alignment fixture boundary; subsequent work must preserve Format Independence. If a construct is not expressible with existing EmbX, demonstrate the smallest concrete gap before changing the language.


## Multidimensional array view boundary — CLOSED

Decoded multidimensional arrays are exposed at the runtime boundary as a non-owning view over contiguous canonical elements plus resolved shape metadata. The conceptual fields are data reference, element count, element size, dimension count and dimensions. This does not introduce a tensor object into EmbX. The target API owns lifetime/ownership details; arbitrary strides and transpose semantics remain outside the core. See `docs/MULTIDIMENSIONAL_ARRAY_VIEW.md`.
