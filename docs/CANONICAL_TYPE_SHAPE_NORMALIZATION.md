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
