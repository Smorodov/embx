# Canonical Type Shape Normalization

## Rule

After AST → IR lowering, all array/length shape semantics live in `core::Type::dimensions`. The AST retains only the source-level modifier list; it has no parallel legacy `Field::length` slot.

There is no second semantic field-length representation.

Examples:

```text
u16[N]       → Fixed(N)
u16 [N]      → Dynamic(N) when the syntax is a field length modifier
bytes[*]     → Remaining
```

The distinction between `Fixed`, `Dynamic` and `Remaining` is semantic, not merely syntactic.

## Plan materialization

PlanBuilder recursively flattens statically resolvable type aliases for execution types and canonicalizes statically evaluable fixed dimensions to literal expressions.

Runtime-dependent expressions remain in Plan with their canonical SymbolId references.

## Consequences

- encoder, decoder, layout and code generation consume one canonical type shape;
- parser-only modifier details do not leak into execution;
- duplicate semantic length state is prohibited;
- a backend never has to reconstruct array shape from AST syntax.


## 7. Physical multidimensional order

Shape normalization deliberately does not create a second storage-order representation. Once dimensions are canonicalized, their order has a normative physical meaning: dimension 0 is outermost and the last dimension varies fastest. The complete rule is defined in `MULTIDIMENSIONAL_ARRAY_LAYOUT.md`.

This separation is important: `core::Type::dimensions` describes both the logical shape and the ordered nesting of that shape, while external tensor stride conventions are handled at the format boundary rather than added to the canonical EmbX type.


## Multidimensional runtime representation

`core::Type::dimensions` remains the only canonical shape representation. The compiler does not introduce a second tensor or stride representation.

When a future runtime/backend API needs to return a decoded multidimensional array, the preferred target-neutral model is a non-owning array view over the contiguous logical element sequence plus the resolved shape. This does not change the canonical type or Plan contract. See `MULTIDIMENSIONAL_ARRAY_VIEW.md`.
