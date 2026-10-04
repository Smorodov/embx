# Lesson 6 — Variants and conditionals

This lesson introduces runtime selection using the existing conditional and variant semantics.

It covers two related mechanisms:

1. `if (...) { ... } else { ... }` selects conditional fields from an expression evaluated against values already available at runtime.
2. `variant name by discriminator { ... }` selects one payload by the discriminator value.

Both mechanisms are executed through the existing Plan, encoder, and decoder. The lesson adds no format-specific runtime machinery.

## Example

```embx
struct Packet big {
  kind: u8;
  variant body by kind {
    1: u16;
    2: { value: u32; }
    default: u8;
  }
}

struct ConditionalPacket big {
  flags: u8;
  if (flags == 1) {
    value: u32;
  } else {
    small: u16;
  }
}
```

For `Packet` with `kind = 1` and variant value `0x1234`:

```text
01 12 34
```

For `Packet` with `kind = 2` and member value `0x11223344`:

```text
02 11 22 33 44
```

For an unmatched `kind = 9`, the default `u8` payload `7` is encoded as:

```text
09 07
```

For `ConditionalPacket` with `flags = 1` and `value = 0x11223344`:

```text
01 11 22 33 44
```

For `flags = 0` and `small = 0x5566`:

```text
00 55 66
```

## What to observe

1. A conditional branch contributes only when its expression is true.
2. A variant selects its case from the discriminator at execution time.
3. A variant with `default` remains valid when no explicit case matches.
4. Decoding reconstructs the selected variant as a nested `Value::Object` under the variant name.
5. The containing structure does not need a second layout or execution mechanism.
