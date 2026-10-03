# Lesson 3 — Bit fields

This lesson introduces `bits` blocks: several integer fields sharing one storage unit.

The example uses one byte split into four fields:

```text
version : 3 bits
kind    : 2 bits
enabled : 1 bit
code    : 2 bits
```

For the values `version=5`, `kind=2`, `enabled=1`, `code=1`, the encoded byte is:

```text
101 10 1 01 = 10110101 = 0xB5
```

The lesson demonstrates the complete course path: source → Plan → reference encode → concrete bytes → reference decode.

## Language rule

Bit fields use the existing `bits` construct. A bit field is an integer scalar with a width in parentheses. The widths in a block must form a valid storage unit; the current language contract limits a block to at most 64 bits.

Bit fields do not accept ordinary field modifiers such as `little`; byte order is controlled by the containing bit storage operation.

## What to observe

1. The four logical fields occupy one byte.
2. The declaration order determines the bit sequence used by the existing Plan/runtime semantics.
3. Encoding and decoding are symmetric.
4. No special runtime mechanism is introduced for the course.
