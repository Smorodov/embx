# Multidimensional Array View Contract

## Status

**Status:** architectural boundary contract for future runtime/backend APIs.

**Language version:** EmbX 0.9.53 accepted green baseline.

**Implementation status:** conceptual contract; no new runtime API is introduced by this document.

This document defines how a decoded multidimensional array may be exposed to a final program without making the EmbX language responsible for a matrix/tensor object model.

## 1. Purpose

EmbX describes and decodes binary data. A final application may need to consume a multidimensional array as a raw contiguous buffer together with its shape rather than as a nested language/container type.

The recommended boundary representation is therefore a **non-owning array view**:

```text
ArrayView
    data              -> pointer to first element
    element_count     -> total number of logical elements
    element_size      -> encoded/in-memory size of one element
    dimension_count   -> number of dimensions
    dimensions        -> extents in canonical EmbX dimension order
```

The exact C++, Rust or Python spelling is backend-specific and must not become part of the language semantics.

## 2. What the view means

For an EmbX array with dimensions:

```text
[D0, D1, ..., D(n-1)]
```

the view describes one contiguous logical element sequence of:

```text
D0 * D1 * ... * D(n-1)
```

elements, subject to the existing checked 64-bit layout rules and runtime allocation limits.

`dimensions` retains the canonical EmbX order: dimension 0 is outermost and the last dimension varies fastest.

The view therefore contains enough information for a final program to reconstruct any application-specific multidimensional abstraction without EmbX choosing that abstraction.

## 3. Example

For:

```text
u16[2][3]
```

with logical values:

```text
[[1, 2, 3],
 [4, 5, 6]]
```

the view conceptually contains:

```text
 element_count   = 6
 element_size    = 2
 dimension_count = 2
 dimensions      = [2, 3]
 data            -> 1, 2, 3, 4, 5, 6
```

The final program may interpret that view as a matrix, tensor, span, image plane, protocol-specific object or plain byte range. EmbX does not prescribe the choice.

## 4. Non-owning lifetime

The view is metadata plus a pointer/reference to storage owned elsewhere.

The owner must remain alive while the view is used. The view must not imply ownership, allocation, deallocation, copying or resizing.

The exact ownership mechanism is backend-specific. A backend may instead return an owning decoded object and expose an array view over it; that does not change the language contract.

## 5. Contiguity

The basic EmbX multidimensional-array contract describes a contiguous logical element sequence at the wire boundary. The view therefore does not require an arbitrary stride vector.

If an application library needs a strided or transposed representation, it may construct its own view or wrapper from the canonical data. Such a representation is outside EmbX semantics.

EmbX must not silently attach application-defined strides to the canonical array view.

## 6. Shape versus physical storage

The view deliberately separates:

- `dimensions` — logical shape;
- `element_count` — total logical element count;
- `element_size` — size of one element;
- `data` — physical contiguous storage.

The view does not itself encode:

- row-major/column-major flags;
- arbitrary strides;
- transpose state;
- alignment metadata;
- tensor quantization semantics;
- application-specific matrix/tensor types.

Those concepts either follow from the EmbX contract or belong to the consuming application/external format.

## 7. Element byte order

For scalar elements wider than one byte, byte order is determined by the existing EmbX type/field byte-order semantics. The view does not add a second byte-order mechanism.

The pointer therefore refers to the decoded representation chosen by the backend. A backend may expose host-native scalar objects or retain an explicitly encoded byte representation, but it must document that choice for the target API. It must not alter the logical element order.

## 8. Dynamic dimensions

A runtime-sized dimension is materialized in the view after its value is known.

For example:

```text
u32[rows][cols]
```

produces a view whose `dimensions` contains the resolved runtime extents. The language remains responsible for decoding the count according to the Plan; the consuming program remains responsible for what it does with the resulting view.

`[*]` remaining sequences retain their existing boundary/termination semantics and are not reinterpreted as a general tensor-view mechanism.

## 9. Reference Runtime and generated backends

The Reference Runtime remains the semantic reference implementation. It must preserve the canonical element order while materializing or exposing data.

A future backend may expose a view directly when that is the natural target representation. Another backend may use nested containers internally and expose a view only at an API boundary.

Both are conforming if they produce the same logical shape and element sequence at the EmbX boundary.

This is a representation decision, not a new semantic layer.

## 10. No tensor abstraction in the core

EmbX core must not acquire a generic tensor/matrix class merely to support multidimensional arrays.

In particular, the following are not introduced by this contract:

- `TensorType` in the semantic core;
- tensor-specific AST nodes;
- tensor-specific Plan operations;
- a stride-aware layout engine;
- matrix algebra;
- transpose/reshape operations;
- external-library dependencies.

A final program may implement any of these above the EmbX boundary when needed.

## 11. GGUF/GGML implication

For GGUF/GGML, the format-specific dimension convention and stride interpretation are resolved before exposing the data as an EmbX array view.

The required sequence is:

```text
GGUF TensorInfo
      |
      v
GGML dimension/stride semantics
      |
      v
explicit mapping to EmbX dimensions
      |
      v
canonical EmbX element sequence
      |
      v
ArrayView
      |
      v
application-specific tensor/matrix representation
```

A GGML tensor does not force EmbX to become a tensor language. The external mapping may reorder metadata interpretation where required, while the final EmbX view remains canonical.

## 12. API design rule

When a backend eventually exposes multidimensional arrays through a public API, prefer the smallest representation that carries the required facts:

```text
pointer/reference + element count + element size + dimension count + dimensions
```

Do not add a field merely because a particular consuming library has one. Add a field only when it represents a stable EmbX semantic fact or an unavoidable ownership/safety requirement of the target API.

If a target requires strides, retain them in the target-specific wrapper rather than promoting them into EmbX unless a concrete language requirement proves that the semantic model itself needs strides.

## 13. Acceptance criteria for a future implementation

A backend/runtime implementation of this boundary is acceptable when:

1. shape is copied from canonical `core::Type::dimensions`/resolved Plan facts;
2. element order matches `MULTIDIMENSIONAL_ARRAY_LAYOUT.md`;
3. `element_count` is checked against the resolved extents;
4. no hidden transpose or dimension reversal occurs;
5. no second semantic layout algorithm is introduced;
6. lifetime/ownership is explicit in the target API;
7. target-specific strides remain outside the EmbX semantic core;
8. exact-byte and round-trip tests prove the logical sequence.

Until a backend implementation is required, this document is a design boundary rather than an implementation obligation.
