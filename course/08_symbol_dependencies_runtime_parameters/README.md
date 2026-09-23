# Lesson 8 — Symbol dependencies and runtime parameters

This lesson connects two existing parts of the EmbX semantic contract:

1. module-level runtime parameters receive canonical `SymbolId`s;
2. executable expressions refer to those symbols directly from the resolved Plan.

The example uses two runtime parameters:

- `count` controls the size of a byte array;
- `gap` controls an explicit `$next + gap` offset.

The parameters are not wire fields. They are supplied in the execution-local parameter environment and are resolved once into the Plan.

## Example

```embx
param count: u8;
param gap: u8;

struct ParameterPacket big {
  values: u8[count];
  at($next + gap) {
    marker: u8;
  }
}
```

With `count = 3`, `gap = 1`, values `[11 22 33]`, and marker `AA`, the wire bytes are:

```text
11 22 33 00 AA
```

The zero byte is the gap before the explicitly positioned marker.

## What the test proves

- both parameters exist in the Plan with `SymbolKind::Parameter`;
- their identities are canonical `SymbolId`s;
- the array dimension refers to the `count` parameter;
- the offset expression refers to the `gap` parameter;
- encoder and decoder use the supplied execution-local parameter environment;
- changing the parameter values changes execution without changing the compiled Plan;
- no second resolver or parameter-specific layout engine is introduced.

The lesson therefore exercises the existing compiler → Plan → reference runtime path rather than adding lesson-specific semantics.
