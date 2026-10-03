# EmbX MIDI Real-Format Example

The MIDI example is an integration fixture, not a new MIDI-specific language feature.
It demonstrates that existing EmbX constructs are sufficient to describe the fixed
structural layer of a Standard MIDI File (SMF) Type 0 file.

## Covered by EmbX

- `MThd` and `MTrk` fixed signatures
- big-endian integer fields
- header length and track count
- a dynamic array of named `Track` values driven by the preceding `track_count`
- bounded track payloads using `block ... [length]` and `bytes[*]`
- preservation of raw MIDI event bytes for a later event-level layer

## Fixture

`examples/midi_demo.mid` is a valid Type 0 SMF with one track, 480 ticks per quarter
note, a 120 BPM tempo event, four notes:
C4, E4, G4 and C5.

Run `examples/play_midi.cmd` on Windows to play the fixture through the Windows MIDI
sequencer API. This avoids relying on a file-association/player choice. The `midi_example_test` CTest verifies that the same real file is decoded
through the EmbX compiler and decoder and that its structural values and event payload
are preserved.

## Deliberate boundary

The schema does not introduce a variable-length integer or running-status mechanism.
Those are MIDI event semantics and are intentionally kept outside the language core.
The course MIDI corpus can build on the raw track payload without changing the
structural contract demonstrated here.

## Text metadata in the fixture

The demo track contains a standard MIDI Track Name meta event with the ASCII text `EmbX`.
EmbX intentionally treats the event stream as raw bytes at this structural layer. The lesson uses
the metadata to demonstrate human-readable information inside a real binary file without creating
a MIDI-specific string type or parser in the language core.
