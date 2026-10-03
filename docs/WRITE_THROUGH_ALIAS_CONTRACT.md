# EmbX 0.9.33 — Write-through Field Alias Contract

**Status:** Accepted in 0.9.46 and retained in the 0.9.48 green baseline.
## Scope

0.9.33 extends the accepted 0.9.32 field-alias construct with a minimal encoder-side write-through input path. It does not introduce transforms or a second storage model.

## Source form

```embx
struct Packet {
  value: u8;
  alias value2 = value;
}
```

The alias target must already be declared and remains identified by its canonical `SymbolId`.

## Encoder semantics

- The physical target field remains the only wire-producing operation.
- Supplying the alias name supplies the target field's scalar input value.
- When the alias input is present, the target field input may be omitted.
- Alias input is bound to the target `SymbolId` before sequential member execution, so the normal field operation remains responsible for encoding and validation.
- If both target and alias inputs are supplied, their scalar values must agree. A disagreement is an encoding error.
- A write-through alias target must be a physical `Field`; a virtual field cannot be a writable alias target.
- Unsupported non-scalar alias input values are rejected rather than silently converted.

## Decoder semantics

Decoder behavior is unchanged from 0.9.32: the physical target is decoded normally and the alias is materialized as a read-only derived member. No additional bytes are consumed.

## Layout and Plan

Aliases remain zero-width layout members and do not create a second layout algorithm. Their own `SymbolId` remains distinct from the target `SymbolId`; the alias-to-target dependency remains explicit in Plan/LayoutGraph.

## Non-goals

- field transforms;
- arbitrary writable virtual fields;
- alias-to-later-field forward references;
- additional wire representation for aliases;
- generated C++ alias setter APIs.

## Acceptance

Acceptance requires a clean MSYS2/UCRT64 configure/build, canonical examples, the complete CTest suite, and documentation/code audit. The 0.9.33 acceptance result is 55/55 CTest tests passed.
