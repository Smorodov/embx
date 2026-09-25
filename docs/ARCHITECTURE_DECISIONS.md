# EmbX Architecture Decisions

## D001 — Plan is the executable contract

The validated Plan is the common semantic representation for runtime, codecs, reflection and backends.

## D002 — SymbolId is canonical identity

Names are contextual presentation/API data. Internal identity is SymbolId-based.

## D003 — One NameResolver

Semantic name resolution is implemented by one canonical resolver over SymbolTable and Scope.

## D004 — Identity and dependency legality are separate

A reference may identify a later declaration while dependency validation rejects the use where declaration order makes it illegal.

## D005 — Plan contains execution facts

Static type shape, static layout and compile-time values are finalized before runtime whenever their operands are known.

## D006 — Runtime retains only dynamic work

Runtime evaluates input-dependent expressions, reads/writes bytes, performs runtime bounds checks and executes callbacks.

## D007 — No VM/bytecode layer

The current `core::Expr` tree is sufficiently small and directly executable. A second instruction format will be introduced only if a concrete requirement demonstrates that it is necessary.

## D008 — 64-bit logical layout

Logical layout quantities use checked unsigned 64-bit semantics. Host memory remains constrained by checked `size_t` conversion and allocation limits.

## D009 — Backend neutrality

Target-language types, ABI conventions and output syntax belong after Plan. Backends do not redefine language semantics.

## D010 — Public names resolve once

Public APIs may accept source names for usability, but resolution happens once at the entry boundary. Internal execution continues with SymbolId.

## D011 — Plan remains intentionally non-serialized

A serial interchange format is deferred until an actual use case justifies freezing one.

## D012 — Deterministic compiler output

Compiler-produced SymbolIds and execution metadata are deterministic for identical source/configuration. Unordered containers never define semantic order.

## D013 — Cleanup is a correctness requirement

The active tree contains one current architecture. Historical compatibility, fallback and repair mechanisms are not retained merely for provenance.


## D014 — One canonical multidimensional array storage order

EmbX uses one normative contiguous multidimensional array order. Dimension 0 is the outermost dimension, the last dimension is the innermost dimension, and the last dimension varies fastest in physical element order. The logical index-to-linear mapping is defined in `MULTIDIMENSIONAL_ARRAY_LAYOUT.md`.

The decision is intentionally explicit but does **not** add a language-level `row_major`, `column_major` or `stride` construct. External formats with different dimension conventions must be mapped explicitly at the format boundary.

This preserves one canonical shape representation (`core::Type::dimensions`), one layout model and one encoder/decoder traversal. A future alternative order requires a concrete requirement and a complete semantic contract before implementation.


## D015 — Multidimensional arrays cross the runtime boundary as views, not tensor objects

The semantic core describes multidimensional array shape and canonical element order, but it does not own a generic matrix/tensor abstraction. A future runtime/backend API may expose decoded multidimensional data as a non-owning array view consisting conceptually of a data pointer/reference, total element count, element size, dimension count and resolved dimensions.

The exact API type is target-specific. Ownership/lifetime is explicit at the target boundary. Arbitrary strides, transpose state, tensor quantization and matrix operations remain outside EmbX semantics. A target may build such abstractions above the view without adding a second semantic model.

The normative boundary is defined in `MULTIDIMENSIONAL_ARRAY_VIEW.md`. This decision is a representation rule, not a new language feature.

## D016 — Universal array buffer boundary

EmbX runtime array data is described by a minimal `ArrayDescriptor` and a separate non-owning `ArrayBuffer`. The descriptor carries resolved dimensions, checked element count, element size and the minimum element identity needed at the runtime boundary. The buffer carries only storage and its host-visible byte size.

The model applies equally to primitive arrays and arrays of named structures. It does not copy the semantic type or Plan into runtime metadata and does not introduce a second array type system. `ArrayView` is the composition of these two facts for non-owning consumers.

The Reference Runtime may continue to use `value::Value::Array`; the descriptor/view is an additional representation boundary, not a replacement. External formats such as GGUF must map their metadata into this boundary explicitly rather than adding format-specific tensor types to EmbX core.
