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
