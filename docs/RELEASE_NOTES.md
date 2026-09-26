# EmbX 0.9.59 — Source Corpus Round-Trip Foundation (candidate)

- Adds a repository-wide `source_roundtrip_corpus_test`.
- Automatically discovers every `.embx` source under `examples/` and `course/` (24 sources in the current baseline).
- Verifies `source → AST → canonical source → AST → canonical source`.
- Requires the two canonical generated sources to be byte-identical.
- Includes imported-source examples without adding import semantics to the generator.
- Adds no language semantics and no second parser, AST, resolver, or semantic model.

# EmbX 0.9.58 — Format Reporter — ACCEPTED GREEN

The Format Reporter adds a deterministic human-readable view over canonical Plan/Reflection facts. It uses the existing LayoutGraph for dependency facts and reports facts not established by the canonical model as `unknown`; it does not introduce a second semantic or layout engine.

- added `src/report/FormatReporter.*`;
- added `--report-format <file>` to the CLI;
- added the normative `docs/FORMAT_REPORTER_CONTRACT.md`;
- added deterministic reporter coverage and updated the CLI contract test;
- preserved AST, Semantic, IR, Plan, Runtime and external-format boundaries unchanged.

The clean target-environment validation passed at **82/82 CTest tests (100%)**. This stage is accepted green.

# EmbX 0.9.57 — canonical Source Generator — ACCEPTED GREEN

This accepted stage begins the post-GGUF Source Generator stage from the accepted 0.9.56 baseline. The implementation reconstructs deterministic canonical EmbX source from the AST and does not introduce a second semantic, expression or layout model.

- added `docs/SOURCE_GENERATOR_CONTRACT.md` as the normative generator boundary;
- clarified that Reference Runtime means the canonical host execution subsystem (`Encoder` + `Decoder` + runtime primitives), not a separate interpreter;
- added `src/codegen/SourceGenerator.*`;
- added `--generate-source <file>` to the CLI;
- added deterministic/idempotence and current-corpus source-generator tests;
- preserved the accepted 0.9.56 GGUF adapter and language/Plan semantics unchanged.

The user validation gate passed at **81/81 CTest tests (100%)** in the target environment. The stage is accepted green.

# EmbX 0.9.56 — GGUF external adapter stage 15.3 — ACCEPTED GREEN

Stage 15.3 expands GGUF external-adapter conformance coverage without changing EmbX core. The candidate adds non-square 3-D mapping, multiple aligned tensors, nested metadata arrays, unsupported-version rejection, arithmetic-overflow and tensor-data-bound checks.

Previous accepted baseline: EmbX 0.9.55 — GGUF external adapter stage 15.2.

Lesson 15 continues without changing the EmbX language or semantic core. This stage closes the explicit GGML↔EmbX dimension-order mapping at the adapter boundary and adds a small valid non-square GGUF fixture.

- GGUF `general.alignment` is now parsed directly by the adapter and its required type/value constraints are checked.
- GGUF tensor offsets are checked against the resolved alignment.
- Fixed-size tensor views now reverse GGML dimension order at the external boundary because GGML dimension 0 is fastest-varying while EmbX's last dimension is fastest-varying.
- Added `course/15_gguf/gguf_2x3_mapping.gguf`, containing two non-square F32 tensors with distinct values for byte-level mapping checks.
- Added negative coverage for invalid alignment metadata and unaligned tensor offsets.
- No GGUF-specific type, stride, layout, AST or Plan mechanism was added to EmbX core.

The clean local validation is complete: **80/80 CTest tests (100%)**. The stage is accepted green.

## Architecture audit closure

The 0.9.55 audit confirms that GGUF-specific identifiers occur only in the external adapter, its test, CMake registration and Lesson 15 documentation/fixtures. EmbX `src/` contains no GGUF/MIDI/other external-format semantics. The Format Independence rule is now explicit in the architecture and vision documents.

# EmbX 0.9.54 — GGUF external adapter stage 15.1 — ACCEPTED GREEN

EmbX 0.9.54 begins Lesson 15 without teaching the language anything about GGUF. The new GGUF adapter is isolated from the semantic core and consumes the existing universal array-buffer boundary.

- Added a standalone GGUF v3 structural parser for header, metadata structure, tensor descriptors, alignment and checked file bounds.
- Added adapter-owned GGUF metadata and tensor type representations.
- Added mapping of fixed-size, unquantized scalar tensors to `ArrayDescriptor` + `ArrayBuffer`.
- Quantized GGUF representations remain format-specific and are not added to EmbX core.
- Added malformed-input and architectural-boundary tests.
- Updated Lesson 15 documentation to make the external-format boundary explicit.

The 0.9.53 79/79 baseline remains the predecessor reference point. The 0.9.54 stage adds one adapter test target and is accepted at 80/80 CTest tests (100%) after clean local validation.

# EmbX 0.9.53 — universal array buffer boundary — ACCEPTED GREEN

EmbX 0.9.53 closes the universal `ArrayDescriptor + ArrayBuffer + ArrayView` runtime boundary. The existing language and Plan semantics remain authoritative; the array layer provides a target-neutral interpretation boundary without introducing a second type or layout system.

- added `src/runtime/Array.h`;
- added checked `ArrayDescriptor`, `ArrayBuffer` and non-owning `ArrayView`;
- added `array_buffer_contract_test`;
- arrays of named structures are explicitly covered by the descriptor contract;
- no tensor/matrix/stride semantics were added to the language;
- existing multidimensional array order remains unchanged;
- GGUF remains a future adapter over this boundary.

# EmbX 0.9.53 multidimensional-array documentation freeze

This documentation-only update preserves the 0.9.52 implementation and language version. No compiler feature is added.

## Documentation closure

- Added the normative `MULTIDIMENSIONAL_ARRAY_LAYOUT.md` contract.
- Explicitly defined EmbX multidimensional physical traversal and index-to-linear mapping.
- Separated shape, dimension order, element order, scalar byte order, alignment and tensor stride semantics.
- Added architecture decision D014 and architecture contract P.
- Updated canonical type-shape, architecture, language and completion-matrix documentation.
- Removed stale roadmap entries for obsolete Lessons 15–17 and made GGUF the sole next Lesson 15.
- Updated the GGUF course documentation to require an explicit GGML↔EmbX dimension mapping and non-square byte-level tensor fixtures.
- Preserved the rule that no `row_major`, `column_major`, arbitrary `stride`, GGUF-specific AST/Plan types or format-specific runtime machinery are introduced without a demonstrated language gap.
- Added `MULTIDIMENSIONAL_ARRAY_VIEW.md`, defining a target-neutral non-owning view boundary for decoded multidimensional data.
- Added D015 and contract Q to make the array-view boundary explicit without adding a tensor abstraction to the semantic core.
- Added `DEVELOPMENT_HANDOFF.md` as a compact continuation guide for Lesson 15.

## External reference

For GGUF/GGML conformance, the current upstream references are the GGUF specification and GGML tensor documentation. The implementation work must still treat the supplied `docs/GGUF_SPECIFICATION.md` as the project-local external format reference.

## 0.9.53 implementation-02

- connected `ArrayDescriptor` to canonical Plan types through `plan::makeArrayDescriptor()`;
- derives element kind, `SymbolId`, fixed encoded element size, dimensions and checked element count from Plan data;
- supports both primitive and named-struct array elements without introducing a second type system;
- added Plan integration coverage for `u32[2][3]` and `Packet[2][3]`;
- the descriptor remains metadata-only and does not own the data buffer.


## 0.9.53 implementation-03

- connected the array-buffer boundary to an actual encoded multidimensional array of named structures;
- verifies `Named[2][3]` descriptor metadata against the canonical Plan;
- verifies encoded contiguous storage size and deliberately distinct element bytes;
- verifies the existing last-dimension-fastest traversal contract without introducing a new layout abstraction;
- keeps `Value::Array`, Encoder and Decoder semantics unchanged.

## 0.9.53 implementation-04

- added checked multidimensional `ArrayView::elementAt()` access;
- verifies rank, bounds, canonical last-dimension-fastest indexing and host-size conversion;
- added dynamic-dimension descriptor coverage through the existing `SymbolEnvironment`;
- added nested named-structure element-size coverage through the existing Plan layout calculation;
- kept `Value::Array`, Encoder, Decoder and semantic type definitions unchanged.


## 0.9.53 implementation-05 — array conformance closure

The universal array-buffer model is now conformance-closed. Coverage includes 2D and 3D shapes, dynamic/fixed dimension combinations, named-structure arrays, nested structures, buffer-size consistency, checked shape multiplication, rank and bounds validation, and canonical last-dimension-fastest indexing.

Acceptance: **79/79 CTest tests (100%)**.

This is the frozen 0.9.53 array-buffer baseline. GGUF is the next integration stage and must consume this boundary rather than introduce tensor-specific semantics into EmbX core.
