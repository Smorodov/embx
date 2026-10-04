# Multidimensional Array Layout Contract

## Status

**Contract status:** normative for the current EmbX implementation and required for all future work involving multidimensional arrays.

**Language version:** EmbX 0.9.53 accepted green baseline.

**Language change:** none. This document records and closes the existing semantic behavior; it does not add `row_major`, `column_major`, `stride`, tensor types, or format-specific constructs.

**Purpose:** make the relationship between array shape, dimension order, logical indexing, physical element order, byte order, alignment, and external tensor formats explicit before GGUF conformance work continues.

## 1. Core distinction

EmbX must distinguish the following concepts:

1. **Shape** — the number of dimensions and the extent of each dimension.
2. **Dimension order** — the order in which dimensions are written in the type's `dimensions` vector and in source syntax.
3. **Logical index** — an index tuple such as `(i0, i1, ..., in-1)` selecting one logical element.
4. **Physical element order** — the order in which elements are emitted to or consumed from the byte stream.
5. **Element byte order** — the byte order used inside a multi-byte scalar element.
6. **Alignment/padding** — gaps inserted between layout regions; these are not array elements.
7. **Stride** — a physical byte distance used by a tensor representation to locate adjacent logical elements along a dimension.

These concepts are related but are not interchangeable. In particular, **byte order does not define multidimensional element order**.

## 2. Canonical EmbX shape

The canonical semantic representation of array shape is `core::Type::dimensions`. There is no second length/shape representation.

For a type with dimensions:

```text
D0, D1, ..., D(n-1)
```

EmbX preserves this order from the canonical type shape into Plan, Reference Runtime, encoder/decoder and generated C++ handling.

For each dimension, the current language supports the existing `Fixed`, `Dynamic` and applicable `Remaining` semantics documented in `CANONICAL_TYPE_SHAPE_NORMALIZATION.md` and `LANGUAGE.md`.

## 3. Normative physical order

The current EmbX execution model is recursive from the first dimension to the last dimension:

```text
D0 -> D1 -> ... -> D(n-1) -> element
```

At each array level, all elements of the current dimension are processed in increasing index order before advancing the enclosing dimension.

Therefore, for a fully materialized multidimensional array:

- dimension 0 is the **outermost** logical dimension;
- the last dimension is the **innermost** logical dimension;
- the last dimension varies fastest in the contiguous byte stream;
- indices are traversed lexicographically with the first dimension as the outer loop.

For a 2 × 3 array:

```text
A[0][0] A[0][1] A[0][2] A[1][0] A[1][1] A[1][2]
```

are emitted in exactly that order.

For a 2 × 3 × 4 array:

```text
A[0][0][0] ... A[0][0][3]
A[0][1][0] ... A[0][1][3]
A[0][2][0] ... A[0][2][3]
A[1][0][0] ... A[1][0][3]
A[1][1][0] ... A[1][1][3]
A[1][2][0] ... A[1][2][3]
```

This is the conventional **row-major traversal** when the first source dimension is regarded as the outer/row dimension. The contract does not depend on the name: the explicit index traversal above is normative.

## 4. Index-to-linear mapping

For extents `D0, D1, ..., D(n-1)`, the zero-based logical index tuple `(i0, i1, ..., i(n-1))` has linear element index:

```text
L = (((i0 * D1 + i1) * D2 + i2) * ... * D(n-1)) + i(n-1)
```

Equivalently, the last dimension has unit element stride and each preceding dimension has a stride equal to the product of all following extents.

For 2 × 3:

```text
L = i0 * 3 + i1
```

For 2 × 3 × 4:

```text
L = i0 * 3 * 4 + i1 * 4 + i2
```

The calculation is logical element order. The physical byte offset additionally depends on the encoded element size and any enclosing layout/alignment rules.

All intermediate layout arithmetic is subject to the existing checked `uint64_t` layout rules.

## 5. What is and is not part of the contract

### Part of the current EmbX contract

- dimension 0 is the outermost dimension;
- the last dimension varies fastest;
- elements are processed in increasing index order at every dimension;
- encoder and decoder use the same traversal;
- generated C++ follows the same logical nesting and traversal;
- array shape is represented only by `core::Type::dimensions`;
- no implicit transpose or dimension reversal occurs inside EmbX.

### Not part of the current EmbX contract

- arbitrary non-contiguous tensor strides;
- views into another tensor with custom strides;
- transposed/permuted storage views;
- column-major storage as an alternative language-level layout;
- a user-selectable array storage-order attribute;
- format-specific tensor semantics.

These are deliberately outside the language until a concrete requirement demonstrates that the existing contract is insufficient.

## 6. Fixed, dynamic and nested arrays

The storage-order rule is independent of whether an extent is fixed or runtime-derived.

For example, if the dimensions are `[rows][cols]` and `rows` is dynamic while `cols` is fixed, the runtime still processes:

```text
row 0: col 0 ... col(cols-1)
row 1: col 0 ... col(cols-1)
...
```

The same rule applies recursively to arrays of structures and structures containing arrays.

`[*]` remaining sequences are a separate termination/boundary mechanism and must not be interpreted as a general multidimensional stride facility.

## 7. Symmetry requirement

For a type with a given canonical shape, encoder and decoder must agree on the same logical-to-physical order.

A valid round trip therefore requires:

```text
Value
  -> Encoder
  -> bytes
  -> Decoder
  -> equivalent Value
```

without an implicit dimension permutation.

The Reference Runtime and generated backends must agree with this order. A backend may use a different in-memory container representation, but that representation must preserve the canonical logical order at the wire boundary.

## 8. Relation to byte order

Consider a two-dimensional array of `u16` values.

There are two independent questions:

1. Which element comes first: determined by the multidimensional traversal contract.
2. Which byte comes first inside that `u16`: determined by the field/type byte-order semantics.

Changing little-endian to big-endian therefore changes the bytes within each element but does **not** change the order of the elements.

## 9. Relation to alignment and padding

Array element traversal is not changed by alignment.

Alignment may insert padding between enclosing layout regions or at explicitly aligned boundaries, but padding is not an element and must not be counted as part of an array's logical shape or element sequence.

For a contiguous array field with no per-element alignment rule, the physical byte range of the array is the concatenation of its element encodings in canonical order.

## 10. External tensor formats: mandatory mapping step

An external format may use a different dimension convention from EmbX. Therefore an external tensor must **never** be mapped by copying its dimension list blindly into an EmbX multidimensional type.

The mapping must separately establish:

1. external dimension names and order;
2. external logical index convention;
3. external physical element order or stride convention;
4. element byte order;
5. any blocking/quantization rules;
6. whether the external representation is contiguous or strided.

Only after these are known may an EmbX type be constructed.

## 11. GGUF / GGML mapping rule

GGUF `TensorInfo` stores a dimension count and a dimension array, but the GGUF container itself does not store a stride vector. The external GGML tensor semantics define the interpretation of tensor dimensions and strides.

Current upstream GGML documentation explicitly states that multidimensional tensors are stored in row-major order and that `ne` contains dimension sizes while `nb` contains byte strides. GGML's canonical contiguous stride calculation makes dimension 0 the fastest-varying dimension.

This means that **GGML's dimension numbering must not be assumed to have the same nesting interpretation as EmbX's source dimensions**.

For Lesson 15, the required procedure is:

```text
GGUF TensorInfo dimensions
        |
        v
GGML dimension convention / stride semantics
        |
        v
explicit mapping to EmbX dimensions
        |
        v
EmbX canonical traversal
```

A 2-D GGUF/GGML tensor with dimensions `(N0, N1)` must be tested with known values so that the physical byte sequence proves whether the chosen EmbX declaration represents the same logical tensor or its transpose.

The mapping is a conformance concern, not a reason to add a GGUF-specific array layout feature to EmbX.

## 12. No premature column-major or stride feature

The current contract intentionally defines one deterministic contiguous order. It does not expose a language-level choice between row-major and column-major storage.

Likewise, EmbX does not currently expose arbitrary strides.

A future proposal for either feature must first provide:

- a concrete external format or user requirement;
- a minimal example that cannot be represented by the current contract;
- an explanation of why dimension permutation at the format boundary is insufficient;
- a complete semantic rule for AST, Plan, runtime, encoder/decoder and all backends;
- conformance tests and documentation.

Until such evidence exists, adding layout-order or stride syntax would violate the project's minimality/sufficiency rule.

## 13. Required conformance tests

Before GGUF tensor work is considered closed, the test suite must include at least:

1. 2 × 3 scalar array with exact bytes and exact logical indices;
2. 3 × 2 scalar array, to prevent accidental symmetry from hiding a transpose;
3. 2 × 3 × 4 scalar array;
4. fixed × dynamic multidimensional array;
5. nested structures containing multidimensional arrays;
6. little-endian multi-byte elements in a multidimensional array;
7. encoder/decoder round-trip for every case;
8. generated C++ byte-for-byte agreement for deterministic cases;
9. a GGUF tensor fixture whose external dimension convention is deliberately checked against known values;
10. a negative/conformance case proving that an unintended dimension reversal is detected rather than silently accepted.

## 14. Decision summary

The current EmbX language therefore has **one normative contiguous multidimensional storage order**:

> The first dimension is outermost, the last dimension is innermost, and the last dimension varies fastest in physical element order.

This is sufficient for the current language and keeps the semantic core minimal. The GGUF capstone must treat external tensor dimension conventions as an explicit mapping problem rather than modifying EmbX to imitate GGML.
