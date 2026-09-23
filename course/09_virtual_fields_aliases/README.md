# Lesson 9 — Virtual fields and aliases

This lesson shows two semantic projections that do not introduce duplicate physical layout:

- a `let` virtual field computes a value from earlier members;
- an `alias` exposes another name for an existing physical field.

Both are resolved through the existing semantic and Plan model. Neither allocates its own
wire bytes.

## Basic example

```embx
struct Projected big {
  value: u8;
  let doubled = value * 2;
  alias value2 = value;
}
```

For `value = 7`, the wire representation is exactly one byte:

```text
07
```

After decode, the logical object exposes:

```text
value   = 7
doubled = 14
value2  = 7
```

## Using both projections in later expressions

The executable lesson also verifies that a virtual field and an alias can participate in
later expressions without introducing a second semantic mechanism:

```embx
struct Projected big {
  value: u8;
  let doubled = value * 2;
  alias length = value;
  payload: bytes[doubled];
  tail: bytes[length];
}
```

For `value = 2`, `payload = 01 02 03 04`, and `tail = 05 06`, the encoded bytes are:

```text
02 01 02 03 04 05 06
```

Here `doubled` supplies the length `4` for `payload`, while the alias `length` supplies
the length `2` for `tail`. The alias still refers to the physical `value` field; it does not
become a second stored field.

## What to observe

1. The virtual field is executable but consumes no bytes.
2. The alias refers to an existing physical field and consumes no bytes.
3. A virtual field can participate in a later expression, including a later array length.
4. An alias can be used as another name for a physical field, including as a later expression input.
5. The alias target remains the canonical physical symbol; no duplicate storage is created.
6. A virtual field or alias cannot bypass the existing declaration-order dependency rules.
7. The reflection surface distinguishes `Virtual` and `Alias` members and preserves the alias target.

The lesson deliberately uses the existing `let` and `alias` semantics. It adds no new runtime
projection mechanism and no second symbol-resolution path.
