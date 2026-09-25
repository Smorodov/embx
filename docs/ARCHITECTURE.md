# EmbX Architecture

## 1. Purpose

EmbX has one semantic pipeline and one executable representation:

```text
source → parser → AST → semantic → IR → Plan → execution/backend
```

The architecture separates source syntax, semantic meaning, executable layout and target-specific implementation.

| Layer | Owns | Must not own |
|---|---|---|
| Grammar/parser | source syntax and parse structure | semantic identity |
| AST | source-oriented structure, names, documentation, spans | runtime state |
| Semantic | name binding, type legality, semantic validation | target ABI |
| IR | self-contained resolved declarations and relationships | parser contexts |
| Plan | validated executable semantics and layout | source reconstruction |
| Runtime | actual execution and runtime values | semantic resolution |
| Backend | target-specific lowering | alternative EmbX semantics |

## 2. AST

AST preserves source-oriented constructs such as type suffixes, field modifiers, documentation, attributes and declaration order. It may contain syntax distinctions that disappear later.

AST lifetime ends at the compiler boundary. No runtime object owns or requires AST nodes.

## 3. Semantic analysis

Semantic analysis validates names, scopes, types, expressions, attributes, callbacks, alias cycles and declaration restrictions. It assigns canonical SymbolIds through the IR-building process.

Name resolution and dependency legality are separate. A forward reference can identify a declaration while later Plan validation rejects an illegal dependency.

## 4. IR

IR is self-contained compiler data. It contains resolved SymbolIds, canonical `core::Type`, canonical `core::Expr`, declaration order and nested operations.

IR declaration order is compiler information. It is used where declaration-order semantics matter, especially constant/enum materialization and dependency validation. It is not copied into Plan as an execution-order mechanism.

## 5. Plan

Plan is the final executable semantic contract. It contains:

- resolved declaration identity;
- canonical execution types;
- normalized dimensions;
- executable expressions with SymbolId references;
- nested field/bit/variant/block/offset/alignment/callback operations;
- static layout values where decidable;
- direct SymbolId-to-Plan indices;
- constants and enum values materialized for execution;
- documentation and attributes needed by reflection/backend output.

PlanBuilder validates identity, dependencies, layout graph, static safety and executable structure before returning a Plan.

## 6. Runtime

Runtime executes Plan plus runtime input/state. It may evaluate an already-resolved expression against runtime values, perform I/O, enforce runtime limits and select runtime-dependent variants.

It must never answer semantic questions such as which declaration a source spelling denotes.

## 7. Backends

The encoder, decoder, reflection layer and C++ generator consume Plan. A backend may choose its own data structures and target syntax, but it may not introduce a second interpretation of EmbX semantics.

## 8. Identity

`SymbolId` is the canonical internal identity. Names remain for diagnostics, reflection and public entry points.

```text
public name
    ↓ one boundary lookup
SymbolId
    ↓
Plan execution
```

## 9. Determinism

Compiler vectors define semantic order. `std::unordered_map` and `std::unordered_set` are used only where iteration order is not semantic or where results are explicitly projected into deterministic vectors.

SymbolIds are deterministic within one compiler-produced symbol table and are not persistent ABI identifiers.

## 9a. Multidimensional array order

Multidimensional array shape is canonicalized in `core::Type::dimensions`. The physical traversal is also canonical: the first dimension is outermost and the last dimension varies fastest. This is defined independently of scalar byte order and alignment.

The architecture does not maintain a separate stride model or tensor object model for ordinary EmbX arrays. At a runtime/backend boundary, decoded multidimensional data may be exposed as a small non-owning array view containing the data reference, element count, element size and shape. External tensor formats must be mapped explicitly to the canonical EmbX order. See `MULTIDIMENSIONAL_ARRAY_LAYOUT.md` and `MULTIDIMENSIONAL_ARRAY_VIEW.md`.

## 10. Layout arithmetic

Logical EmbX layout quantities use checked `uint64_t` semantics. Actual buffer access uses host-sized offsets only after checked conversion. Logical range therefore does not imply unlimited physical memory.

## 11. Completion criterion

No layer is allowed to compensate for a missing upstream semantic fact. If runtime or a backend needs information that belongs to language meaning, the Plan contract must be extended instead.
