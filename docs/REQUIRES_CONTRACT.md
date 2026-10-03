# EmbX `requires` Contract

**Status:** Accepted in 0.9.46 and retained in the 0.9.48 green baseline.

The feature was originally accepted in 0.9.38.

## 1. Purpose

`requires expression;` is a semantic constraint on the completed value of a struct. It is not a layout operation and consumes no wire bytes.

## 2. Syntax

```embx
struct Packet {
  value: u16;
  requires value >= 1 && value <= 100;
}
```

A struct may contain zero or more `requires` declarations.

## 3. Canonical pipeline

```text
Grammar
→ AST
→ Semantic
→ IR
→ Plan
→ Runtime
→ Encoder / Decoder
→ Reflection
→ C++ backend
→ Tests
→ Documentation
```

The executable representation is `core::Expr`; identifiers are bound to `SymbolId`.

## 4. Semantics

A `requires` expression must have canonical expression type `Boolean`.

The expression is evaluated against the completed execution environment of the struct. Therefore it may reference fields, aliases, virtual fields, module constants, enum values and runtime parameters that are available in the completed Plan environment.

A failed requirement is a constraint violation and must fail the operation.

- Decoder: decode the struct value, then evaluate all requirements. On failure, the decode operation rolls back transactionally.
- Encoder: evaluate all requirements after member values have been populated but before committing the encoded struct output. On failure, the transactional writer is rolled back and no output is committed.

Requirements do not alter layout, size bounds, offsets, alignment, byte order or `$next` semantics.

## 5. Type and dependency rules

- The result type must be `Boolean`.
- Expression resolution uses canonical `SymbolId` identity.
- Unknown identifiers are rejected during Plan construction.
- Requirement evaluation uses the existing canonical expression evaluator.
- No second requirement evaluator is introduced.

Because requirements are evaluated after the complete struct environment exists, their references are not constrained by source declaration order inside the struct. They still must resolve to declared semantic value symbols.

## 6. Reflection

Reflection exposes the canonical source expression text for each struct requirement. Reflection does not introduce a second executable representation.

## 7. C++ backend

The generated C++ backend reproduces the Plan requirement expression for the supported generated value surface. Requirements are checked before committing encoded output and after successful field decoding.

Layout built-ins such as `$next` and generated-size properties are not exposed as generated C++ object fields; a requirement using such a value is rejected by the generated backend rather than silently changing semantics.

## 8. Non-goals

This closure does not introduce:

- a separate assertion AST;
- a runtime name lookup mechanism;
- a layout graph algorithm for requirements;
- a second expression evaluator;
- `$default` byte-order syntax;
- implicit conversion rules beyond the canonical expression subsystem.
