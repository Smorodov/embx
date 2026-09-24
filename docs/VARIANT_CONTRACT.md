# EmbX Variant Contract

## Stage 3 — Variant Closure

**Status:** Accepted in 0.9.46 and retained in the 0.9.48 green baseline.

This contract was originally introduced and accepted in 0.9.39.

`variant` uses the canonical expression, SymbolId, Plan, LayoutBounds, LayoutGraph, Encoder and Decoder mechanisms.

### Dispatch

The discriminator is a canonical runtime-capable `core::Expr`. Case tags are compile-time integer expressions. During Plan construction they are evaluated once and materialized as integer `runtime::Value` values. Runtime never evaluates case-tag expressions.

Dispatch is therefore:

```text
evaluate discriminator
→ compare with materialized Plan tags
→ execute selected payload
→ otherwise default
→ otherwise VariantMismatch
```

### Tags

Tags must be integer values compatible with the discriminator domain. Negative values are forbidden for unsigned discriminators. Values outside the discriminator domain are rejected. Duplicate detection is performed on materialized numeric values, not source spelling.

### Layout

Variant layout uses existing `LayoutBounds`, `SequenceBounds` and checked arithmetic. The minimum is the minimum branch size; the maximum is the maximum branch size, or unbounded if any reachable branch is unbounded.

### Payload

Typed and inline member-sequence payloads use the existing type/member semantics. Nested variants are recursive uses of the same Plan and runtime mechanisms.

### Default

At most one default branch is allowed. Without a matching case and without a default, execution fails with `VariantMismatch`.

### Backend

The reference runtime is closed first. Generated C++ support must consume the executable Plan and must not resolve names, evaluate case tags, inspect AST/IR, or introduce a second semantic evaluator.
