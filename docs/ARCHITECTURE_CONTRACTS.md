# EmbX Architecture Contracts

These contracts are normative for the current tree.

## A. Canonical semantics

`plan::Plan` is the sole validated executable semantic representation.

## B. Canonical identity

`core::SymbolId` / `core::SymbolRef` are the only internal semantic identity. `0` is invalid.

## C. One resolver

`core::NameResolver` is the canonical AST/IR name-resolution mechanism. Public API name lookup is a boundary operation, not an alternative semantic system. Parser namespace/import processing may canonicalize explicitly qualified names, but it must not resolve unqualified lexical value identifiers. Unqualified value binding belongs to `Scope`/`NameResolver`, so child declarations can shadow module declarations.

## D. Identity versus legality

Resolving a declaration and validating whether a dependency is legal are distinct operations.

## E. No semantic repair

Forbidden mechanisms include:

- name-to-index fallback after canonical identity fails;
- synthetic SymbolIds created to repair unresolved references;
- backend-side semantic lookup;
- reparsing source during execution;
- duplicate semantic models with independent rules.

## F. Representation isolation

ANTLR details stop at parser/AST boundaries. AST details stop before persistent IR/Plan execution. Target ABI details remain downstream of Plan.

## G. Ownership

IR and Plan own their semantic data. They do not retain parser-context ownership or borrowed AST state.

## H. Canonical types and expressions

`core::Type` and `core::Expr` are the shared semantic representations. Named types and expression identifiers carry SymbolIds.

## I. Plan completeness

All description-dependent facts required for execution must be finalized in Plan. Runtime receives only the remaining runtime-dependent work.

## J. Runtime isolation

Runtime does not inspect AST, IR or perform semantic name resolution.

## K. Determinism

Semantic order is defined by explicit vectors and source/compiler order. Unordered lookup containers must not define observable semantic order.

## L. 64-bit logical layout

Logical sizes, offsets, extents, counts, alignments and derived sizes use checked unsigned 64-bit semantics. Host-memory conversion is checked separately.

## M. Symmetry

Encoder and decoder implement the same Plan semantics in opposite directions. Direction-specific callbacks are explicit operations, not separate semantic models.

## N. Diagnostics

Diagnostics are compiler/tooling contracts. Runtime errors must not silently turn into semantic repair.

## O. Cleanup integrity

Dead code, duplicate mechanisms, obsolete compatibility paths and stale documentation are defects when they obscure or contradict the current architecture.


## P. Multidimensional array layout

The canonical array shape is `core::Type::dimensions`. For dimensions `D0..Dn-1`, dimension 0 is outermost and dimension `n-1` varies fastest. Encoder, decoder, Reference Runtime and generated backends must preserve this order.

Array storage order is independent of element byte order and alignment. No backend may silently transpose or reverse dimensions. External tensor formats require an explicit dimension/stride mapping before they are represented as EmbX arrays.

The complete normative definition and index-to-linear mapping are in `MULTIDIMENSIONAL_ARRAY_LAYOUT.md`.


## Q. Multidimensional array view boundary

A multidimensional array may be exposed at a runtime/backend API as a non-owning view over contiguous logical elements. The conceptual facts are: data pointer/reference, total element count, element size, dimension count and resolved dimensions in canonical EmbX order.

The view does not introduce tensor semantics, arbitrary strides, transpose state or a second layout model. Ownership and lifetime are target-API concerns. The complete boundary is defined in `MULTIDIMENSIONAL_ARRAY_VIEW.md`.
