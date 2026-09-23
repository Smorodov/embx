# Lesson 7 — Offsets and alignment

This lesson composes the existing layout controls `at(...)`, `$next`, and `align(...)`.

The important distinction is that an `at(...)` region has an explicit physical position,
while the containing sequential cursor continues independently. `$next` refers to the end
of the preceding sequential operation when used in an `at(...)` offset expression.

Inside an `at(...)` region, `align(...)` aligns the region's physical cursor. It may insert
padding bytes before the following member.

## Example

```embx
struct OffsetAlign big {
  head: u8;
  at($next + 2) {
    marker: u8;
    align(4);
    value: u16;
  }
  tail: u8;
}
```

For:

```text
head   = 0x07
marker = 0xAA
value  = 0x1122
tail   = 0x08
```

the encoded bytes are:

```text
07 08 00 AA 11 22
```

The sequential `tail` remains immediately after `head`, while the `at(...)` region starts
after `$next + 2`. The two-byte gap before `marker` is preserved, and `align(4)` leaves
`marker` at offset 3 and places `value` at offset 4.

## What to observe

1. `$next` is evaluated from the sequential layout cursor.
2. `at(...)` places its region at an explicit physical offset without moving the caller's
   sequential cursor past the region.
3. `align(4)` operates on the physical cursor of the active region.
4. Padding is part of the encoded binary layout.
5. No lesson-specific layout mechanism is required: the existing Plan, encoder, and decoder
   execute the same layout semantics used elsewhere in EmbX.
