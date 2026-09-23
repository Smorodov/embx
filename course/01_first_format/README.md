# Lesson 1 — First binary format

This lesson introduces the smallest complete EmbX binary format:

- one fixed-width 16-bit integer;
- one 8-bit integer;
- one fixed four-byte payload;
- a fixed big-endian byte representation;
- encode/decode round-trip through the real EmbX Plan and runtime.

The format is intentionally artificial. Its purpose is to teach the language without
introducing protocol-specific semantics.

## EmbX source

`message.embx` defines:

```text
struct Message big {
  magic: u16 = 0xCAFE;
  version: u8;
  payload: bytes[4];
}
```

The expected object used by the regression test is:

```text
magic   = 0xCAFE
version = 0x01
payload = DE AD BE EF
```

## Binary representation

Because the structure is `big`, the 16-bit `magic` value is encoded as:

```text
CA FE 01 DE AD BE EF
```

The total size is exactly 7 bytes.

## Acceptance chain

```text
message.embx
    ↓
compiler / AST / IR / Plan
    ↓
reference encoder
    ↓
CA FE 01 DE AD BE EF
    ↓
reference decoder
    ↓
original values
```

The test also checks that the Plan reports an exact seven-byte layout.

## Run

Build EmbX normally, then run:

```text
ctest --test-dir build -R lesson_01_first_format --output-on-failure
```

The lesson deliberately does not add any new compiler or runtime mechanism.
