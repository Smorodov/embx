# Lesson 14 — TLV composition

This lesson builds a small Type–Length–Value format from existing EmbX mechanisms.
It deliberately introduces **no TLV-specific compiler semantics**.

```embx
struct Record big {
    kind: u8;
    length: u16;
    data: bytes[length];
}

struct Message big {
    records: Record[*] until 0xFF 0xFF max 4096;
}
```

The wire fixture contains three records followed by `FF FF`:

- kind `1`, length `3`, payload `41 42 43` (`ABC`);
- kind `2`, length `4`, payload `DE AD BE EF`;
- kind `3`, length `0`, empty payload;
- outer sequence terminator `FF FF`.

The lesson demonstrates that a length-dependent byte field and a terminated sequence of
structured elements compose directly. The outer terminator is considered only between
`Record` elements, so `FF FF` occurring inside a record payload does not terminate the list.

The negative fixture declares a payload length larger than the bytes available before the
containing input ends. Decoding must fail transactionally rather than consuming the malformed
record or the following bytes.

## Validation

`lesson_14_tlv` compiles this source into an executable Plan, decodes the real fixture, checks
field values, performs an encode/decode byte-for-byte round trip, verifies an inner `FF FF`
payload does not terminate the outer sequence, and rejects the invalid-length fixture.

The example uses only existing EmbX concepts: integer fields, expression-dependent byte arrays,
remaining sequences, terminated sequences, bounded layout, and transactional decode failure.
