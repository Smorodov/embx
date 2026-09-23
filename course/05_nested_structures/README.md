# Lesson 5 — Nested structures

This lesson introduces composition of already-defined structures.

It uses only the existing named-type semantics:

1. `Header` is a small fixed-size structure.
2. `Packet` contains a `Header` field followed by ordinary fields.
3. The nested structure is encoded and decoded through the same Plan as the containing structure.

## Example

```embx
struct Header big {
  version: u8;
  flags: u8;
}

struct Packet big {
  header: Header;
  sequence: u16;
  payload: u8[3];
}
```

For:

```text
version = 1
flags   = 2
sequence = 0x1234
payload = [0xAA, 0xBB, 0xCC]
```

the encoded bytes are:

```text
01 02 12 34 AA BB CC
```

`Header` contributes exactly 2 bytes. `Packet` therefore has an exact size of 7 bytes.

## What to observe

1. A named structure can be used directly as a field type.
2. The nested structure keeps its own field layout and byte order.
3. The containing structure composes that layout without a second layout mechanism.
4. Encode and decode produce nested `Value::Object` data.
5. The example stays inside the existing Plan and runtime contracts.
