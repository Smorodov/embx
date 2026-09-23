# Lesson 4 — Arrays and dynamic dimensions

This lesson introduces array dimensions using the existing type syntax.

It compares two cases:

1. `u8[4]` — a fixed array whose byte size is known when the Plan is built.
2. `u16[count]` — a runtime-sized array whose element count comes from an earlier field.

The lesson deliberately uses only the existing array and expression semantics. It does not add a new array mechanism.

## Fixed array

```embx
struct StaticArray big {
  values: u8[4];
}
```

The Plan knows the exact size: 4 bytes.

For `values = [1, 2, 3, 4]`, the encoded bytes are:

```text
01 02 03 04
```

## Dynamic array

```embx
struct DynamicArray big {
  count: u8;
  values: u16[count];
}
```

The count is read from the already-declared `count` field. The element width is fixed at 2 bytes, but the number of elements is runtime-dependent.

For `count = 3` and `values = [0x1122, 0x3344, 0x5566]`, the encoded bytes are:

```text
03 11 22 33 44 55 66
```

Because the dimension is runtime-dependent, the current Plan layout contract does not claim a static size or finite maximum for this array. Its minimum size is zero for the array itself; for the complete `DynamicArray` structure the mandatory `count` field gives a minimum of 1 byte.

## What to observe

1. A fixed dimension contributes an exact size to the Plan.
2. A runtime dimension is evaluated during execution and does not require a second layout mechanism.
3. The dimension expression may reference a field declared earlier in the same structure.
4. Encoding and decoding use the same resolved dimension semantics.
5. The course example stays inside the existing compiler/runtime contract.
