# GGUF in EmbX

Lesson 15 uses GGUF as an external-format conformance target. The GGUF parser/adapter is deliberately outside the EmbX language, AST, semantic core, Plan and universal array model. The original format specification remains unchanged in `docs/GGUF_SPECIFICATION.md`.

## 1. Architectural boundary

The governing rule is:

> EmbX describes data structures and their semantics; it does not know the origin, container format, or domain meaning of the bytes.

Therefore GGUF-specific concepts belong only to the adapter:

- GGUF magic and version;
- GGUF metadata value tags;
- GGML tensor type codes and quantization;
- GGUF tensor-data offsets and alignment rules;
- GGML dimension-numbering conventions;
- GGUF metadata naming rules.

Nothing named `GGUF`, `GGML`, `TensorInfo`, `Quantization` or equivalent is added to EmbX AST/Plan/runtime core.

The direction is:

```text
GGUF bytes
    ↓
GGUF parser / adapter
    ↓
universal EmbX boundary
    ├── ArrayDescriptor
    └── ArrayBuffer / ArrayView
    ↓
existing EmbX/runtime semantics
```

The reverse dependency is forbidden: EmbX core must not depend on the GGUF adapter.

## 2. Stage 15.1 — structural parser

The first implementation stage parses only the external GGUF structure:

1. magic;
2. version;
3. tensor count;
4. metadata count;
5. metadata keys and recursively skippable metadata values;
6. tensor names;
7. tensor dimensions;
8. GGML tensor type code;
9. tensor-data offset;
10. alignment and file-bound checks.

The parser records GGUF metadata as adapter-owned information. It does not translate metadata into EmbX attributes or compiler constructs.

The initial implementation is explicitly little-endian and version-3 only, matching the project-local frozen GGUF reference. Endianness handling remains a GGUF concern and must not leak into universal array semantics.

## 3. Universal array mapping

Only after structural parsing succeeds may the adapter expose a tensor through the existing universal array boundary.

For fixed-size, unquantized scalar tensor types, the adapter can produce:

```text
GGML dimensions (dim0 fastest)
    ↓ reverse at the external boundary
EmbX ArrayDescriptor.dimensions (last dimension fastest)
GGUF scalar representation → element size
tensor-data file region → ArrayBuffer
```

The adapter does not add a GGUF type to `ArrayDescriptor`. Quantized representations remain GGUF-specific because their storage is block-encoded and cannot be honestly represented as a fixed-size scalar element merely by naming a new core type.

## 4. Dimension mapping

GGUF/GGML dimension numbering is an external convention. It must be mapped explicitly to EmbX's canonical contiguous order rather than changing EmbX's array model.

EmbX remains:

```text
dimension 0 = outermost source-level dimension
last dimension = fastest varying
```

The adapter must prove any GGML↔EmbX mapping with non-square byte-level fixtures such as 2 × 3 and 3 × 2. No `row_major`, `column_major` or arbitrary-stride syntax is introduced for GGUF.

## 5. Test boundary

Structural adapter tests cover:

- valid minimal GGUF;
- metadata scalars and nested arrays;
- multiple tensors;
- 2-D and 3-D dimensions;
- invalid magic;
- unsupported version;
- truncated header/metadata;
- invalid metadata type;
- invalid dimension/rank;
- unaligned tensor offset;
- tensor offset outside the file;
- arithmetic overflow.

Universal mapping tests cover fixed-size scalar tensors and explicitly verify that quantized GGUF representations do not become new EmbX semantic types.

## 6. Dimension mapping fixture

`gguf_2x3_mapping.gguf` is a small valid GGUF v3 fixture with two F32 tensors. Its tensor dimensions are deliberately non-square:

- GGML tensor A: `[2, 3]` → EmbX view shape `[3, 2]`;
- GGML tensor B: `[3, 2]` → EmbX view shape `[2, 3]`.

The fixture stores distinct scalar values so the mapping can be checked at the byte level. It is a project-local conformance fixture, not an EmbX language construct.

The external Python `gguf` package may be used later to generate independent golden fixtures. It is a fixture producer/oracle only. It is not an EmbX dependency and must not influence the language design.

## 7. Completion criterion

GGUF integration is complete only when the adapter can consume real GGUF files while the EmbX core remains format-agnostic. A need to add GGUF-specific fields, AST nodes, Plan operations, runtime types or layout evaluators is treated first as an architectural defect or a demonstrated language gap, not as an automatic feature request.
