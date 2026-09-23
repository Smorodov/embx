# Lesson 13 — Standard MIDI File

This lesson uses a real Standard MIDI File rather than a synthetic language-only example.
The goal is to demonstrate that EmbX can describe a real binary container while keeping
format-specific event semantics outside the language core.

## 1. Header

The demo is SMF Type 0:

```text
4D 54 68 64   MThd
00 00 00 06   header length = 6
00 00         format = 0
00 01         tracks = 1
01 E0         division = 480 ticks/quarter
```

All multi-byte header values are big-endian, which is expressed directly by the `big`
struct declaration.

## 2. Track

The track chunk is:

```text
4D 54 72 6B   MTrk
00 00 00 2F   47 bytes
```

The event stream begins with a 120 BPM tempo event and then four C-major arpeggio notes.
The demo deliberately omits program-change and other optional events so playback depends
on as little external MIDI state as possible.

## 3. EmbX boundary

`examples/midi.embx` describes:

- fixed `MThd` and `MTrk` signatures;
- big-endian integers;
- the track count;
- a repeated `Track` structure;
- a length-bounded block;
- raw event bytes.

MIDI variable-length quantities, running status and event semantics remain ordinary bytes
at this stage. They are not special cases in the EmbX core.

## 4. Verification

Build the project and run the normal examples and CTest suite. Then run:

```text
course\midi_corpus\run_corpus.cmd
```

On Windows, run:

```text
examples\play_midi.cmd
```

The playback helper uses the Windows MIDI sequencer API instead of depending on which
application is associated with `.mid` files.

## 5. What this lesson proves

The same fixture is checked at three independent levels:

1. byte-level MIDI conformance corpus;
2. EmbX compiler/reference-decoder round trip;
3. audible playback on Windows.

A future lesson can move from raw track bytes to explicit MIDI event semantics without
changing the structural contract demonstrated here.
