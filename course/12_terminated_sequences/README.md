# Lesson 12 — General terminated sequences

This lesson introduces EmbX terminated sequences as a **general binary sequence mechanism**.
It is not a string feature: an element may be raw bytes or a structured type, and a sequence
may itself be nested inside another terminated sequence.

## Source

`terminated_sequences.embx` defines two levels:

```embx
struct Entry {
    value: bytes until 0x00 max 8;
}

struct EntryList {
    entries: Entry[*] until 0xFF 0xFF max 32;
    tail: u8;
}
```

The inner sequence terminator is `00`. The outer sequence terminator is `FF FF`.
The outer terminator is considered only **between complete `Entry` elements**. A `00` belongs
to the `Entry` currently being decoded; it cannot terminate the outer sequence.

## Fixture

`terminated_sequences.bin` contains:

```text
41 00 42 43 00 FF FF 7F
```

It represents two entries:

```text
Entry 1 payload: 41   00
Entry 2 payload: 42 43   00
Entry list:       FF FF
Tail:             7F
```

The payload is deliberately small, but the example is binary rather than string-specific.
The terminators are protocol bytes with no character-set interpretation.

## What the test proves

`lesson_12_terminated_sequences_test` checks that:

1. the lesson source compiles into an executable Plan;
2. the nested terminated sequence decodes the two structured elements correctly;
3. the outer terminator is recognized only at an element boundary;
4. encoding the decoded value reproduces the exact fixture bytes;
5. a missing outer terminator fails transactionally.

Generated C++ conformance for the same generalized terminated-sequence contract is covered by
the core generated-code regression suite; this lesson does not add lesson-specific compiler
semantics.

## Reproducibility

From the build directory:

```text
ctest --test-dir build -R lesson_12_terminated_sequences --output-on-failure
```

This lesson is part of the **0.9.50 course update** built on the accepted 0.9.49 language baseline.
