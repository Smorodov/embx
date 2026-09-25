# EmbX Development Handoff — 0.9.53 Array Buffer Implementation

## Purpose

This document is the short operational handoff for continuing EmbX development from the 0.9.53 array-buffer implementation checkpoint. It is intentionally redundant with the normative contracts where that redundancy makes the next work unambiguous.

## Current state

- Implementation baseline: **EmbX 0.9.53**.
- Accepted course state: **Lessons 1–14**.
- Accepted local test gate: **79/79 CTest tests (100%)**.
- 0.9.53 closes the universal array-buffer boundary with descriptor/buffer/view separation, Plan integration, dynamic dimensions, nested structures and conformance coverage; existing Plan and Value semantics remain unchanged.
- The next implementation stage is Lesson 15 — GGUF, using the frozen array-buffer boundary as its tensor-data representation boundary.

## Non-negotiable architecture

```text
source
  -> parser
  -> AST
  -> semantic analysis
  -> IR
  -> Plan
  -> Reference Runtime / codecs / reflection / backends
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
8. Reference Runtime/generated C++ agreement;
9. array-view metadata agreement once an implementation is introduced.

Implementation-05 completes the conformance matrix, including dynamic/fixed combinations, nested structures, checked shape arithmetic and bounds/rank validation. The concrete non-owning `ArrayView::elementAt()` is already part of the accepted runtime boundary.
boundary, including rank/bounds checks and checked canonical index mapping.
Dynamic dimensions are resolved through the existing runtime environment, and
nested named structures use the existing Plan fixed-size calculation.

Use distinct values so transpose errors cannot hide in symmetric fixtures.

## Lesson 15 — GGUF work order

1. Start from the smallest valid GGUF header.
2. Add strings and scalar metadata.
3. Add tagged metadata arrays and investigate nested arrays.
4. Add TensorInfo and dimension arrays.
5. Add alignment, tensor-data region and offsets.
6. Generate golden fixtures with the external Python `gguf` package.
7. Decode through Reference Runtime and generated C++.
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

The next implementation step is dynamic dimensions and nested-structure coverage. Do not introduce tensor, matrix, stride, or format-specific abstractions.
