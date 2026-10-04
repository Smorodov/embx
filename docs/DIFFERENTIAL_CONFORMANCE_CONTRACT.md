# Differential Conformance Contract

## Purpose

The generated C++ backend is an implementation of the executable semantics represented by `plan::Plan`.
The canonical host execution path is the Reference Runtime, composed of the existing Encoder, Decoder, and runtime primitives.

Differential conformance compares both paths on the same compiled Plan semantics.

## Required comparisons

### Encode

`Plan + Value -> Reference Runtime -> bytes A`

`Plan -> C++ Generator -> generated codec -> bytes B`

For supported cases, `bytes A == bytes B`.

### Decode

`bytes -> Reference Runtime -> Value A`

`bytes -> generated codec -> Value B`

The observable fields and consumed byte count must agree.

## Scope

The first corpus is intentionally small and deterministic. It exercises a fixed scalar structure with explicit big-endian layout. Additional cases should be added only when the comparison mechanism itself is stable.

This layer does not add language constructs, format-specific semantics, or a second semantic engine.


## RuntimeParameters ABI invariant

For modules with runtime parameters, generated `RuntimeParameters` contains every parameter present in the validated Plan. The C++ generator must not silently omit a validated parameter. The runtime-parameter differential test exercises the generated `count` and `delta` fields.
