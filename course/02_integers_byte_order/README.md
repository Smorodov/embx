# Lesson 2 — Integers and byte order

This lesson explains how EmbX represents integer values on the wire and how byte order is selected.

It introduces three related ideas:

- fixed-width unsigned integers;
- signed integers;
- explicit big-endian and little-endian byte order.

The lesson deliberately uses a small artificial format. No protocol-specific rules are needed.

## EmbX source

`message.embx` defines one structure:

```embx
struct Numbers big {
  bigValue: u16;
  littleValue: u16 little;
  signedLittle: i16 little;
  byteValue: u8;
}
```

The structure default is `big`, so `bigValue` and `byteValue` use big-endian semantics where byte order is relevant.
`littleValue` and `signedLittle` explicitly override that default.

The test uses:

```text
bigValue     = 0x1234
littleValue  = 0x5678
signedLittle = -2
byteValue    = 0xA5
```

## Binary representation

The resulting bytes are:

```text
12 34 78 56 FE FF A5
```

The first `u16` is big-endian:

```text
0x1234 → 12 34
```

The second `u16` is little-endian:

```text
0x5678 → 78 56
```

The signed `i16` value `-2` is represented in two's-complement form and then written little-endian:

```text
-2 → 0xFFFE → FE FF
```

The final `u8` occupies one byte, so byte order has no visible effect:

```text
0xA5 → A5
```

The complete layout is therefore exactly seven bytes.

## Byte-order precedence

The current EmbX language resolves effective byte order using this precedence:

```text
module default
      ↓
struct override
      ↓
field override
```

This lesson uses the struct-level default plus field-level overrides. The compiler resolves the effective order into the Plan; the runtime does not perform a second byte-order resolution.

Supported source values are `little`, `big` and `native`.

## Acceptance chain

```text
message.embx
    ↓
compiler / AST / IR / Plan
    ↓
reference encoder
    ↓
12 34 78 56 FE FF A5
    ↓
reference decoder
    ↓
original signed and unsigned values
```

The test also checks the exact seven-byte Plan layout and the decoded signed value.

## Run

Build EmbX normally, then run:

```text
ctest --test-dir build -R lesson_02_integers_byte_order --output-on-failure
```

The lesson does not add compiler or runtime mechanisms. It exercises the existing integer and byte-order semantics.
