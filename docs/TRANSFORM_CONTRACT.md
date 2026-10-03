# EmbX 0.9.34 — Transform Contract

**Status:** Accepted in 0.9.46 and retained in the 0.9.48 green baseline.
Transform is value conversion attached only to a physical Field. It adds zero layout bytes and creates no LayoutGraph node.

## Syntax
```embx
struct Packet {
    temperature: i16 transform scale(0.1);
}
```

## Semantics
Decoder: `logical = wire * factor`. Encoder: `wire = logical / factor`. Only `scale(factor)` is standardized in 0.9.34. The factor is a finite, non-zero compile-time numeric constant. Runtime parameters and arbitrary transform expressions are not allowed.

Transforms are forbidden on aliases and virtual fields. A write-through alias uses the transform of its physical target.

Transforms do not change field size, offset, alignment, min/max size, or layout dependencies. Reference Encoder/Decoder execution is normative; generated C++ transform execution remains outside the 0.9.34 accepted backend subset and is tracked in the language audit.

## Numeric exactness

The logical result of `scale` is represented as a binary64 floating value. Consequently, transformed `i64`/`u64` wire values are accepted by the reference decoder only when the wire integer is exactly representable as binary64; larger integers are rejected rather than silently rounded. Encoding to `f32` requires an exact binary32 round-trip; silent narrowing is rejected. Integer wire bounds are checked before conversion.
