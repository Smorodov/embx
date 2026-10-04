# EmbX Development Handoff — 0.9.85 GREEN — fixed1

## Purpose

This document is the short operational handoff for continuing EmbX development from the user-verified 0.9.83 GREEN source state through the 0.9.84 audit and into the next controlled development phase. It is intentionally redundant with the normative contracts where that redundancy makes the next work unambiguous.

## Current state

- Last accepted source state: **EmbX 0.9.84**, based on user-verified 91/91 GREEN 0.9.83 source and all four build profiles.
- Current public-preparation source state: **EmbX 0.9.86**, based on accepted 0.9.85 GREEN; packaging/documentation milestone, not a 1.0.0 claim.
- Current phase: **post-C++-generator audit and controlled continuation**.
- Accepted course state: **Lessons 1–15**.
- Accepted baseline test gate: **91/91 CTest tests (100%)**. Full acceptance remains `build_all.cmd`.
- 0.9.53 closes the universal array-buffer boundary with descriptor/buffer/view separation, Plan integration, dynamic dimensions, nested structures and conformance coverage; existing Plan and Value semantics remain unchanged.
- Lesson 15.2 GGUF is accepted as an external adapter using the frozen array-buffer boundary as its tensor-data representation boundary.

## Open work after the 0.9.84 audit

### 0.9.85 GREEN slice

Generated C++ now accepts callbacks inside conditional branches. The differential corpus combines this with an existing scalar `scale` transform and checks Reference Runtime ↔ generated C++ agreement for encode/decode, callback arguments and exact wire bytes. No new CTest target is introduced.

The language itself has no mandatory unfinished feature phase. The remaining work is: generated-C++
differential qualification of already implemented combinations, runtime/error/transactional
hardening, property/fuzz testing where useful, conformance-corpus expansion, generated-code
quality/performance review, and eventual release hardening.

Additional generated-language backends (Rust, Python and others) are deliberately outside the
mandatory roadmap and are left to the community. New language constructs require a demonstrated
universal semantic gap rather than a feature-count goal or a single-format requirement.

## Non-negotiable architecture

```text
source
  -> parser
  -> AST
  -> semantic analysis
  -> IR
  -> Plan
  -> Reference Runtime (Encoder/Decoder + runtime primitives) / reflection / backends
```

Keep these invariants:

1. Plan is the executable semantic boundary.
2. SymbolId is canonical internal identity.
3. One NameResolver and one expression semantic model.
4. One canonical type-shape model in `core::Type::dimensions`.
5. One canonical multidimensional array order.
6. No backend-side semantic repair.
7. No second layout engine.
8. Checked 64-bit logical layout arithmetic.
9. External formats do not create format-specific semantic machinery.
10. Remove stale/duplicate mechanisms instead of preserving them for compatibility without a demonstrated need.

## Multidimensional array decision

For dimensions `[D0, D1, ..., Dn-1]`:

- dimension 0 is outermost;
- the last dimension varies fastest;
- elements are contiguous in canonical logical order at the wire boundary;
- shape is represented only by `core::Type::dimensions`;
- scalar byte order is independent of element order;
- alignment/padding is independent of logical array shape;
- arbitrary stride, transpose and column-major language modes are not part of EmbX.

The normative documents are:

- `docs/MULTIDIMENSIONAL_ARRAY_LAYOUT.md`
- `docs/MULTIDIMENSIONAL_ARRAY_VIEW.md`

## Runtime/backend representation decision

The runtime now exposes decoded multidimensional data through a **non-owning `ArrayView`** rather than forcing a nested container or tensor object.

Conceptually:

```text
ArrayView
    data/reference
    element_count
    element_size
    dimension_count
    dimensions[]
```

This is a boundary representation, not a new semantic type. Ownership and lifetime remain explicit at the API boundary; the view itself does not own the backing buffer.

Do not add a tensor class, stride engine, transpose operation or GGUF-specific array type to the core merely to implement this view.

## Accepted multidimensional conformance before GGUF tensor closure

At minimum prove:

1. 2 × 3 exact element order and bytes;
2. 3 × 2 exact element order and bytes;
3. 2 × 3 × 4 exact order;
4. fixed × dynamic dimensions;
5. nested structures containing arrays;
6. scalar byte-order independence from element order;
7. encoder/decoder symmetry;
8. Reference Runtime (Encoder/Decoder + runtime primitives) / generated C++ agreement;
9. array-view metadata agreement once an implementation is introduced.

Implementation-05 completes the conformance matrix, including dynamic/fixed combinations, nested structures, checked shape arithmetic and bounds/rank validation. The concrete non-owning `ArrayView::elementAt()` is already part of the accepted runtime boundary.
boundary, including rank/bounds checks and checked canonical index mapping.
Dynamic dimensions are resolved through the existing runtime environment, and
nested named structures use the existing Plan fixed-size calculation.

Use distinct values so transpose errors cannot hide in symmetric fixtures.

## Current next stage

The 0.9.61 differential layout expansion is closed at **87/87 CTest tests (100%)**. The 0.9.62 terminated-sequence expansion is closed at **88/88**, the 0.9.63 runtime-parameter differential slice is closed at **89/89**, and the 0.9.64 access slice is closed at **90/90**. The subsequent generated virtual-field/accessor and codec-contract closure is accepted in 0.9.69 at **91/91**. Virtual fields, field aliases and scalar `scale` transforms are qualified against the canonical host execution path. No language feature was added. No language feature is planned solely to expand this matrix.

The permanent rule remains: if a differential failure appears, localize it first to Plan, Reference Runtime, generated backend, or test fixture. Do not introduce a language mechanism merely to make a backend or external format pass.

## Historical post-GGUF work order

1. Start from the smallest valid GGUF header.
2. Add strings and scalar metadata.
3. Add tagged metadata arrays and investigate nested arrays.
4. Add TensorInfo and dimension arrays.
5. Add alignment, tensor-data region and offsets.
6. Generate golden fixtures with the external Python `gguf` package.
7. Decode through the canonical Reference Runtime and generated C++.
8. Compare exact bytes where representation is deterministic.
9. Add negative fixtures.
10. Perform a deep audit before Source Generator / Format Reporter work.

For tensors, never copy GGUF dimensions blindly into EmbX. Establish the GGML dimension/stride convention first, then map it explicitly to the EmbX canonical order.

## External GGUF dependency boundary

The Python `gguf` package is an external oracle/fixture producer/validator. It is not an EmbX semantic dependency and must not leak into AST, IR, Plan or runtime semantics.

## What not to do now

Do not introduce:

- `row_major` or `column_major` language syntax;
- arbitrary `stride` syntax;
- tensor-specific AST/IR/Plan types;
- GGUF-specific runtime/layout evaluators;
- a generic matrix/tensor semantic core;
- a second expression evaluator;
- a second resolver;
- a second layout algorithm.

If a real format proves an existing construct insufficient, first create the smallest reproducible source/fixture/test demonstrating the gap. Only then decide whether language evolution is justified.

## 0.9.53 conformance closure

The descriptor boundary is now exercised against a real encoded multidimensional array of named structures. The test verifies `Plan::Type -> ArrayDescriptor`, the encoded contiguous buffer, element size/count, and the existing last-dimension-fastest traversal contract.

The next substantive implementation step should be selected from a concrete semantic/backend gap demonstrated by a contract and regression test. Do not introduce tensor, matrix, stride, or format-specific abstractions merely for convenience.


## 0.9.54 GGUF adapter acceptance

Lesson 15.1 is accepted as an external-format adapter. The full source suite is validated at **80/80 CTest tests (100%)**. GGUF-specific concepts remain outside EmbX core.

## Test profile workflow

See `docs/TEST_PROFILES.md` for the normal, extended, audit and full-GREEN build commands. The profile labels are CTest metadata only; all 91 tests remain registered and `build_all.cmd` executes the complete suite.
