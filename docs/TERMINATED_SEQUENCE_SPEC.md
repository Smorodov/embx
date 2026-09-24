# EmbX Terminated Sequences

**Status:** Normative design specification for EmbX 0.9.49
**Predecessor:** EmbX 0.9.48 accepted green

## 1. Purpose

A terminated sequence is a general binary sequence whose end is represented by a terminal byte sequence rather than by an explicit element count or byte length. It is not a string-specific feature.

The element may be a scalar, structure, variant, fixed-size sequence, or another valid self-delimiting type. Existing `bytes until ... max ...` remains valid as the compact byte-payload form.

## 2. Syntax

General sequence form:

```embx
Element[*] until TERMINATOR max MAX_PAYLOAD;
```

Example:

```embx
struct Item {
  value: u8;
}

struct List {
  items: Item[*] until 0xFF 0xFF max 4096;
}
```

Existing byte form remains valid:

```embx
data: bytes until 0x00 max 256;
```

No `cstring`, `zstring`, `terminated_string`, or other string-specific type is introduced.

## 3. Terminator

A terminator is a non-empty byte sequence and is matched byte-for-byte.

Examples:

```embx
until 0x00
until 0x0D 0x0A
until 0xFF 0xFF
```

## 4. Element-boundary rule

The terminator of a sequence is recognized only at an element boundary of that sequence.

Conceptually:

```text
save cursor

repeat:
    if terminator matches at cursor:
        consume terminator
        succeed

    decode one element

    if element decoding fails:
        restore cursor
        fail
```

An implementation must not perform a raw search for the first terminator and consume everything before it when the sequence contains structured elements.

## 5. Nested sequences

Nested terminated sequences are valid.

```embx
struct Entry {
  value: bytes until 0x00 max 16;
}

struct EntryList {
  entries: Entry[*] until 0xFF 0xFF max 64;
}
```

The `0x00` terminator belongs to `Entry`. The `0xFF 0xFF` terminator belongs to `EntryList`. The outer terminator is checked only between complete `Entry` elements.

Thus:

```text
[Entry][Entry][FF FF]
```

is valid, and an inner `00` does not terminate the outer sequence.

## 6. Terminator bytes inside elements

Bytes equal to the outer terminator may occur inside the encoded representation of an element. They do not terminate the outer sequence unless the decoder is at an outer element boundary.

This rule prevents nested element semantics from being confused with raw-byte searching.

## 7. Empty sequence

A terminated sequence may contain zero elements.

For:

```embx
items: Item[*] until 0xFF 0xFF max 4096;
```

`FF FF` represents an empty sequence and the terminator is consumed.

## 8. Maximum payload

`max` is mandatory and specifies the maximum physical payload before the sequence terminator. The terminator is excluded from the payload limit.

For terminator length `T` and payload limit `N`:

```text
maximum physical extent = N + T
```

The maximum prevents a malformed input from causing an unbounded search.

## 9. Layout bounds

The existing `LayoutBounds` model remains authoritative:

```text
minSize = terminator.size
maxSize = maxPayload + terminator.size
layoutClass = Bounded
```

The minimum occurs when the sequence contains zero elements.

No second layout or size-analysis mechanism is introduced.

## 10. Containing bounds

A terminated sequence may inspect only the readable region established by its containing operation. A terminator outside that region does not belong to the sequence.

The intrinsic `maxPayload + terminator.size` bound and the containing reader/layout limit both apply.

## 11. Decode failure and rollback

Decoding is transactional.

On success the cursor is immediately after the terminator.

On failure the externally visible cursor is restored to its position before the sequence started.

Failure includes:

- missing terminator;
- element decoding failure;
- nested element failure;
- maximum payload exceeded;
- containing-layout boundary reached.

## 12. Element progress

A successful non-terminating iteration must consume at least one byte. The implementation must not permit a zero-progress element to create an infinite loop.

## 13. Encoding

The logical value contains elements only. The encoder writes every element and then appends the terminator exactly once.

The encoded payload must not exceed `maxPayload`.

For the byte form, a logical payload containing the terminator is rejected. No escaping mechanism is introduced.

## 14. AST

Termination is a property of a sequence operation, not a special property of the `bytes` type.

The semantic representation must retain:

- element/base type;
- sequence dimension;
- terminator bytes;
- maximum payload.

The exact AST data structure remains an implementation detail.

## 15. Semantic validation

Semantic analysis validates:

1. a valid element type;
2. a valid sequence shape;
3. a non-empty terminator;
4. a mandatory maximum payload;
5. representability in Plan;
6. derivable layout bounds.

For the generalized form, the current language representation uses exactly one `[ * ]` dimension before `until`.

## 16. Plan

The executable Plan must contain all information required for execution:

```text
element type
terminator
maximum payload
layout bounds
```

Runtime must not inspect source syntax or redo name resolution/semantic analysis.

No separate terminated-sequence IR is introduced.

## 17. Reference Runtime

The Reference Runtime is the normative executable implementation.

Its sequence operation is equivalent to:

```text
save cursor

while true:
    if terminator matches at current element boundary:
        consume terminator
        succeed

    if payload limit is exhausted:
        restore cursor
        fail

    decode one element

    if element decoding fails:
        restore cursor
        fail
```

Implementations may optimize this algorithm provided observable semantics remain identical.

## 18. Generated backends

Generated backends implement the semantics represented by Plan. They must agree with the Reference Runtime on:

- terminator matching;
- element boundaries;
- maximum payload;
- rollback;
- nested sequences.

## 19. Reflection

Reflection derives terminated-sequence information from Plan and existing layout metadata. Where applicable it exposes:

- element type;
- terminator;
- maximum payload;
- minimum size;
- maximum size;
- `LayoutClass::Bounded`.

No reflection-only semantic mechanism is introduced.

## 20. Compatibility

The existing declaration:

```embx
value: bytes until 0x00 max 256;
```

remains valid and retains its existing logical behavior.

Its generalized implementation is allowed to use the same semantic machinery as structured terminated sequences.

## 21. Required conformance cases

The implementation must cover:

- empty sequence;
- one element;
- multiple elements;
- multi-byte terminator;
- exact maximum payload;
- payload overflow;
- missing terminator;
- element failure;
- rollback;
- containing-block boundary;
- structure elements;
- nested terminated sequences;
- independent inner and outer terminators;
- outer terminator checked only between elements;
- Reference Runtime vs generated C++ decode;
- Reference Runtime vs generated C++ encode.

## 22. Minimality constraints

The feature must not introduce:

- string-specific terminated types;
- a second expression system;
- a second type resolver;
- a separate terminated-sequence IR;
- a separate layout engine;
- a separate runtime evaluator;
- backend-specific semantics.

The existing AST → Plan → Reference Runtime/backend architecture remains the source of truth.

## 23. Acceptance criteria

The 0.9.49 implementation is complete when:

- existing byte terminated syntax remains valid;
- structured terminated sequences compile;
- nested terminated sequences execute correctly;
- terminators are recognized only at the correct element boundaries;
- `max` and containing bounds are enforced;
- failures restore the original cursor;
- empty sequences work;
- Reflection remains Plan-derived;
- generated C++ agrees with the Reference Runtime;
- documentation consistently uses **terminated sequence** rather than string-specific terminology;
- the complete test suite is green after a clean build.
