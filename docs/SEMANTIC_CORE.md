# EmbX Semantic Core

## 1. Symbol identity

`core::SymbolId` is a compiler-local opaque identity. `InvalidSymbolId` is zero. `core::SymbolRef` carries that identity through types and expressions.

Module/global declarations enter the SymbolTable name index. Scope-owned declarations, such as fields, receive identity without becoming module-global names.

## 2. Scope

`core::Scope` resolves the current scope first and then its parents. A child binding can shadow a parent binding.

The IR builder predeclares declarations that may participate in references before lowering their bodies. This permits precise forward-reference diagnostics without making every forward dependency legal.

## 3. Name resolution

`core::NameResolver` provides:

- empty-name rejection;
- lexical scope lookup;
- module/global lookup;
- exact qualified lookup;
- optional kind validation.

Parser namespace/import processing may rewrite an explicitly qualified import alias to its canonical qualified spelling. It must not prematurely qualify an unqualified value identifier merely because a module declaration with that spelling exists. The module scope exposes current-module declarations by local spelling so lexical resolution can select a child binding first.

No backend owns another semantic resolver.

## 4. Types

`core::Type` contains:

- `TypeKind` (`Primitive`, `Bytes`, `String`, `Named`);
- source/display name;
- optional SymbolRef for named declarations;
- canonical dimensions.

Named references may denote aliases, structs or enums. Cyclic aliases are rejected.

## 5. Dimensions

The semantic type model distinguishes:

- `Fixed` — an explicit expression in a type suffix;
- `Dynamic` — an explicit runtime-dependent field-length expression;
- `Remaining` — the `[*]` form, with no expression.

Plan materialization canonicalizes statically evaluable dimensions to literal expressions. Genuinely runtime-dependent expressions remain explicit.

## 6. Expressions

`core::Expr` contains a compact tree of literals, identifiers, unary operations, binary operations and parentheses.

For an identifier:

- `reference.id` is execution identity;
- `text` is diagnostic/source spelling.

The runtime SymbolEnvironment evaluator never repairs a missing identity by consulting `text`.

## 7. Compile-time values

Constants and enum values are materialized into Plan. Their dependency analysis uses SymbolIds, not source-name repair.

## 8. Callback semantics

Callback direction is derived from the declared callback name:

- `on_decode...` → decode;
- `on_encode...` → encode;
- any other prefix → invalid declaration/use.

The callback registry is a runtime boundary API; internal expression execution remains SymbolId-based.
