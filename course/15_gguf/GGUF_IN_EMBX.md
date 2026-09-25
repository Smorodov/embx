# GGUF in EmbX

This document restates the supplied GGUF specification in the terminology and composition model used by the EmbX course. The original specification is preserved verbatim as [`docs/GGUF_SPECIFICATION.md`](../../docs/GGUF_SPECIFICATION.md) and remains the external format reference.

## 1. Purpose and boundary

GGUF is a real binary container format. Lesson 15 uses it as a capstone conformance target rather than adding GGUF-specific semantics to EmbX.

The working rule is:

> First express the format with existing EmbX mechanisms. If a part cannot be expressed, record the concrete limitation before changing the language.

The external `gguf` Python package may be used to create and validate golden fixtures. It is an external oracle, not part of the EmbX semantic core.

## 2. Byte order

The supplied GGUF specification states that fields are little-endian by default and documents big-endian support in version 3. The first Lesson 15 fixtures use the little-endian form.

In EmbX this is expressed at the structure/layout level rather than by introducing GGUF-specific integer types:

```embx
struct Header little {
    magic: bytes[4];
    version: u32;
    tensor_count: u64;
    metadata_kv_count: u64;
}
```

A constant magic value can be constrained by the normal EmbX constant mechanism:

```embx
magic: bytes[4] = "GGUF";
```

## 3. GGUF string

The GGUF string representation is a `u64` byte length followed by that many UTF-8 bytes. It is not NUL terminated.

The direct EmbX shape is:

```embx
struct String little {
    length: u64;
    data: bytes[length];
}
```

UTF-8 validity is a value-level constraint and must not be confused with the binary layout itself.

## 4. Metadata value type

GGUF metadata values have a numeric type tag. The supplied specification defines unsigned and signed 8/16/32/64-bit integers, float32/64, bool, string, and array.

The fixed scalar alternatives can be represented as an EmbX variant. The exact variant syntax used in the executable lesson must follow the current compiler grammar; the conceptual model is a tagged union, not a new GGUF primitive.

```text
MetadataValueType  -> existing EmbX variant/tag representation
MetadataValue      -> corresponding existing EmbX variant representation
```

No GGUF-specific `metadata_value` type is added to the compiler.

## 5. Metadata arrays

The supplied specification defines an array as:

1. element type tag;
2. element count;
3. array elements.

For a concrete homogeneous array, the binary relationship is naturally expressed as a dynamic EmbX dimension:

```embx
values: Element[element_count];
```

The important distinction is that the array's element type is encoded in the preceding tag. A tagged value therefore selects which existing structural form follows.

The specification permits nested arrays. Lesson 15 must test this explicitly. If the recursive tagged representation cannot be expressed directly with the current EmbX variant facilities, that fact is a documented investigation result rather than an invitation to add an ad-hoc GGUF mechanism.

## 6. Metadata key/value record

A metadata key is a GGUF string followed by a metadata value type and value. Conceptually:

```embx
struct MetadataKV little {
    key: String;
    value_type: MetadataValueType;
    value: MetadataValue;
}
```

The supplied specification also imposes lexical restrictions on metadata keys (ASCII hierarchical lower_snake_case segments and a maximum encoded length). These are semantic/value constraints, not additional wire-layout primitives.

## 7. Header

The GGUF header consists of the magic, version, tensor count, metadata KV count, and the metadata KV sequence.

The direct course model is:

```embx
struct Header little {
    magic: bytes[4] = "GGUF";
    version: u32;
    tensor_count: u64;
    metadata_kv_count: u64;
    metadata: MetadataKV[metadata_kv_count];
}
```

The lesson should use the actual GGUF producer to create at least one minimal valid header fixture rather than hand-authoring all bytes.

## 8. Tensor information

Each tensor info record contains:

- a GGUF string name;
- `n_dimensions` as `u32`;
- `dimensions[n_dimensions]` as `u64` values;
- tensor type as a GGML type enumeration;
- `offset` as `u64` relative to the tensor-data region.

The direct structural part is:

```embx
struct TensorInfo little {
    name: String;
    n_dimensions: u32;
    dimensions: u64[n_dimensions];
    type: GgmlType;
    offset: u64;
}
```

`GgmlType` is a finite format enumeration. It should be modeled with existing EmbX variant/enum facilities rather than a compiler-level GGUF type.

## 9. Tensor data and alignment

The complete file has the following physical order:

```text
Header
TensorInfo[*]
padding to ALIGNMENT
Tensor data
```

Tensor offsets are relative to the beginning of tensor data. Tensor data itself is arbitrary binary data and is not interpreted by the GGUF container parser.

Alignment belongs to EmbX's existing layout model. Lesson 15 must distinguish:

- the physical alignment/padding of the file;
- the logical tensor offset stored in `TensorInfo`;
- the external interpretation of tensor bytes according to `GgmlType`.

No GGUF-specific alignment primitive should be introduced.

## 10. Multidimensional tensor layout and dimension mapping

GGUF tensor dimensions must not be copied into an EmbX multidimensional declaration without first checking the external GGML dimension convention.

EmbX canonical order is:

```text
D0 outermost -> ... -> D(n-1) innermost
D(n-1) varies fastest
```

Current upstream GGML documentation describes multidimensional tensors as row-major and exposes `ne` for dimension sizes and `nb` for byte strides. Its contiguous stride calculation makes dimension 0 the fastest-varying dimension. Therefore the dimension numbering used by GGML must be mapped explicitly to EmbX's source-level nesting convention.

Lesson 15 must include a 2-D tensor with distinct values (for example 1..6) and verify the exact physical byte order. A square matrix is insufficient because transposition can remain invisible; use both 2 × 3 and 3 × 2 cases.

The lesson must distinguish:

- GGUF dimension metadata;
- GGML logical dimension numbering;
- GGML contiguous/strided physical interpretation;
- EmbX logical nesting;
- scalar element byte order;
- alignment and tensor-data offsets.

No GGUF-specific storage-order or stride feature is added to EmbX. If a concrete tensor cannot be represented by the current contract, record the minimal reproducible gap before proposing language evolution. Do not introduce a generic tensor object into the EmbX core merely to consume GGUF tensors; the application-side representation remains outside the language.

## 11. Counts and widths

The supplied GGUF specification uses `u64` for most countable values. This makes GGUF a useful practical check of the internal width choices already made in EmbX.

Lesson 15 should therefore exercise large-width declarations such as:

```embx
count: u64;
length: u64;
offset: u64;
dimensions: u64[count];
```

The test should validate actual encoded values, not merely compile-time type names.

## 12. Real-fixture conformance

The lesson corpus should contain files produced by the external `gguf` library where practical:

```text
course/15_gguf/corpus/
    minimal.gguf
    metadata.gguf
    arrays.gguf
    tensors.gguf
```

Later negative fixtures may cover:

```text
bad_magic.gguf
truncated_header.gguf
truncated_metadata.gguf
invalid_alignment.gguf
```

The conformance path is:

```text
GGUF specification
        |
        v
    EmbX model
        |
        v
      Plan
     /    \
    v      v
Reference  Generated C++
 Runtime
     \    /
      \  /
       vv
  same external fixture
```

Where encoding is deterministic, the produced bytes should also be compared byte-for-byte with the golden fixture. Where the external library intentionally permits representation choices, compare the decoded semantic values and validate the result with an independent GGUF reader.

## 13. Known investigation points

The following are deliberately kept as investigation points rather than silently resolved:

1. Recursive metadata arrays and their exact expression using current EmbX variants.
2. Tensor data as an arbitrary byte region whose physical location is constrained by alignment and stored offsets.
3. Version-3 big-endian support and whether the fixture corpus should include it.
4. Very large `u64` counts/offsets and their practical interaction with host/container limits.
5. Validation of UTF-8 and metadata-key lexical rules as value constraints rather than layout rules.

A failure to express one of these with current EmbX must first be reproduced with a minimal source and test case. Only then should language evolution be considered.

## 14. Course rule

GGUF-specific terminology belongs in this lesson and its documentation. The compiler, Plan, Reference Runtime, and generated backends remain format-independent.

The desired result is not a special GGUF implementation inside EmbX. It is evidence that existing EmbX composition mechanisms are sufficient to describe a substantial real-world binary format, with any genuine language gaps isolated and documented.
