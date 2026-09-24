# Lesson 11 — IPv4 header

This lesson maps a fixed IPv4 header onto existing EmbX primitives. It deliberately adds **no IPv4-specific compiler or runtime semantics**.

## Source

`ipv4.embx` defines the 20-byte base IPv4 header:

- the first byte is split into 4-bit `version` and 4-bit `ihl` fields;
- `dscp` is one byte;
- `total_length`, `identification`, `flags_fragment`, and `checksum` are big-endian `u16` fields;
- `ttl` is one byte;
- `protocol` is an existing `u8`-backed enum;
- `source` and `destination` are four-byte arrays.

The schema describes the wire layout only. It does not calculate or validate the IPv4 checksum, interpret protocol payloads, or implement IPv4 semantics.

## Fixture

`ipv4_header.bin` is one concrete 20-byte header:

```text
45 00 00 14 12 34 40 00 40 06 3C 79 C0 00 02 01 C6 33 64 02
```

The fields are:

- version = 4
- IHL = 5 (20-byte header)
- DSCP/ECN byte = `00`
- total length = `20`
- identification = `0x1234`
- flags/fragment offset = `0x4000` (DF, offset 0)
- TTL = `64`
- protocol = TCP (`6`)
- header checksum = `0x3C79`
- source = `192.0.2.1`
- destination = `198.51.100.2`

The checksum is part of the fixture, but checksum verification is intentionally performed by the test as an independent property rather than as EmbX language semantics.

## What the test proves

`lesson_11_ipv4_test` checks:

1. the IPv4 source compiles into an executable Plan;
2. the header has an exact 20-byte layout;
3. decoding the real fixture produces the expected field values;
4. encoding the decoded value reproduces the exact fixture bytes;
5. the checksum field in the fixture is consistent with the IPv4 one's-complement header checksum calculation.

## Reproducibility

From the build directory:

```text
ctest --test-dir build -R lesson_11_ipv4 --output-on-failure
```

This lesson is retained in the **0.9.51 accepted green course baseline**. Its IPv4 contract was accepted in the 0.9.47 stage and remains unchanged.
