# Universal Array Buffer Model

## Status

**Status:** accepted implementation contract, EmbX 0.9.53.

EmbX represents array storage at a runtime boundary as two deliberately separate
facts:

```text
ArrayDescriptor + ArrayBuffer
```

The descriptor explains the logical data. The buffer contains the contiguous
bytes. Neither introduces a tensor or matrix type into the semantic core.

## 1. ArrayDescriptor

The runtime descriptor contains the smallest stable set of facts required to
interpret a contiguous array:

```text
ArrayDescriptor
    elementKind
    elementTypeId
    elementSize
    elementCount
    dimensions[]
```

`elementTypeId` is used for named elements and is an identity, not a name lookup
mechanism. The semantic `SymbolId` remains authoritative for named EmbX types.

`elementCount` is the checked product of all resolved dimensions. Its logical
range is `uint64_t`, consistent with EmbX layout arithmetic.

## 2. ArrayBuffer

```text
ArrayBuffer
    data
    size
```

The buffer is non-owning. The owner is responsible for its lifetime. The byte
size is checked at the host boundary using `size_t`.

## 3. ArrayView

`ArrayView` is simply:

```text
ArrayView = ArrayDescriptor + ArrayBuffer
```

It does not own, resize, transpose or reinterpret the storage.

A valid non-empty view satisfies:

```text
buffer.size == elementCount * elementSize
```

with checked conversion to the host addressable size.

## 4. Arrays of structures

The element is not restricted to a primitive type. For example:

```text
struct Packet {
    id: u32
    value: u32
    flags: u32
}

packets: Packet[2][3]
```

is represented by one descriptor whose element kind is `Named`, whose element
identity identifies `Packet`, whose element size is the encoded size of one
`Packet`, and whose dimensions are `[2, 3]`.

The descriptor does not duplicate the fields of `Packet`. Those remain defined
by the canonical EmbX Plan.

## 5. Shape and storage are separate

The descriptor carries logical shape. The buffer carries physical storage.

Therefore the model does not introduce:

- `TensorType`;
- `MatrixType`;
- language-level row/column-major attributes;
- arbitrary strides;
- transpose state;
- tensor quantization;
- matrix operations.

The canonical EmbX multidimensional order remains defined by
`MULTIDIMENSIONAL_ARRAY_LAYOUT.md`.

External formats may map their dimension conventions explicitly at the format
boundary. The mapping must not create a second semantic array model.

## 6. Relationship to Plan and Value

The Plan remains the executable semantic boundary. `ArrayDescriptor` is a
runtime representation of resolved array facts; it is not a replacement for
`core::Type` or `plan::Type`.

The current Reference Decoder/Encoder continue to use `value::Value::Array` as
the semantic value representation. The descriptor/view is an additional
runtime boundary and does not duplicate or replace `Value`.

This allows the implementation to introduce zero-copy or backend-specific array
views later without changing the language value model.

## 7. GGUF boundary

A future GGUF adapter maps GGUF tensor metadata into an `ArrayDescriptor` and a
buffer. GGUF-specific dimension conventions are resolved by that adapter. No
GGUF tensor class is added to EmbX core.

```text
GGUF TensorInfo
      |
      v
explicit format mapping
      |
      v
ArrayDescriptor + ArrayBuffer
      |
      v
application-specific tensor view
```

## 8. Implementation rule

The runtime descriptor must remain smaller than the semantic type system. Add a
field only when it is a stable EmbX fact or an unavoidable host-safety boundary.
Do not copy complete `core::Type`, Plan operations, field definitions or
format-specific metadata into the descriptor.

## 7. ArrayView indexed access

`ArrayView::elementAt(indices, out, error)` provides the minimal read-only
boundary for multidimensional access. It:

- requires the index rank to equal the descriptor rank;
- checks every index against its corresponding extent;
- maps indices using the canonical last-dimension-fastest order;
- computes the byte offset with checked `uint64_t` arithmetic;
- performs the final conversion to host `size_t` only at the physical buffer boundary;
- returns a pointer into the existing buffer and never copies or reallocates data.

This is indexing of the already-defined EmbX layout, not a second layout or
stride system.

## 8. Dynamic dimensions and nested structures

`plan::makeArrayDescriptor()` resolves both fixed and dynamic dimensions through
the existing runtime expression evaluator and execution-local `SymbolEnvironment`.
A dynamic dimension therefore changes only the resolved descriptor shape; it
does not require a new array type or layout engine.

Nested named structures use the existing `fixedTypeSize()` Plan calculation.
The descriptor records the complete encoded size of one outer element while
field structure remains authoritative in the Plan.

## 9. Conformance closure

The runtime boundary is considered conforming only when the same descriptor/view
model covers the following cases without a second array mechanism:

- fixed × fixed shapes such as `[2][3]` and `[3][2]`;
- three-dimensional shapes such as `[2][3][4]`;
- resolved dynamic × fixed, fixed × dynamic, and dynamic × dynamic shapes;
- arrays whose element is a named structure;
- nested named structures;
- zero-length dimensions;
- malformed buffer sizes and element-count inconsistencies;
- checked dimension-product overflow;
- indexed access at first, interior, and final elements;
- out-of-range and wrong-rank indexing.

All of these cases use the same last-dimension-fastest linearization and the same
checked `uint64_t` arithmetic already defined by the runtime boundary. No stride,
transpose, tensor, or format-specific representation is introduced by conformance
testing.
