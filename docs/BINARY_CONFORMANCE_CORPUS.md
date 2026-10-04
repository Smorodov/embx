# Binary Conformance Corpus

## Purpose

The binary conformance corpus fixes observable binary behavior for the existing
EmbX language and Reference Runtime. It is a regression boundary, not a second
semantic specification and not a new layout engine.

For every valid corpus item the test performs:

1. compile the `.embx` source into the executable `Plan`;
2. decode the fixed binary fixture with the Reference Runtime;
3. require complete input consumption;
4. encode the decoded value again;
5. require byte-for-byte equality with the original fixture.

For invalid corpus items the decoder must reject the fixture.

## Current corpus

| Model | Type | Fixture | Expected behavior |
|---|---|---|---|
| Lesson 1 | `Message` | `course/01_first_format/message.bin` | decode + exact re-encode |
| Lesson 11 | `net::ipv4::Header` | `course/11_ipv4/ipv4_header.bin` | decode + exact re-encode |
| Lesson 12 | `EntryList` | `course/12_terminated_sequences/terminated_sequences.bin` | decode + exact re-encode |
| Lesson 14 | `Message` | `course/14_tlv/tlv.bin` | decode + exact re-encode |
| MIDI example | `examples::midi::MidiFile` | `examples/midi_demo.mid` | decode + exact re-encode |
| Lesson 14 invalid | `Message` | `course/14_tlv/tlv_invalid_length.bin` | reject |

## Scope rule

The corpus records behavior already defined by EmbX Plan semantics. Adding a
format-specific fixture must not require a format-specific construct in the
language, AST, IR, Plan, or Reference Runtime.

New binary fixtures should first be added when they expose a concrete semantic
or runtime boundary. The corpus should grow by coverage of existing universal
constructs, not by collecting formats for their own sake.

## Next expansion

The 0.9.59 generated-backend differential layer is now closed and expanded by the 0.9.60 matrix. The next conformance layers are:

- differential coverage for conditional fields, dynamic dimensions, offsets/alignment, runtime parameters, aliases/virtual fields, terminated sequences and callbacks/transforms;
- malformed-input corpus expansion;
- property-based binary generation;
- fuzzing at parser, Plan, decoder, and encoder boundaries.
