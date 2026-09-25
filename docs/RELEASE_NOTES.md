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
