# EmbX — Canonical Examples

The examples are the small executable language corpus used by `run_examples.cmd`.

| File | Main purpose |
|---|---|
| `ipv4.embx` | namespace, enum, bits, arrays and big-endian layout |
| `container.embx` | bounded block, `bytes[*]` and alignment |
| `callback.embx` | decode/encode callback declaration and use |
| `common.embx` | imported namespace and shared definitions |
| `packet_import.embx` | imports, qualified names, alias, block and `at()` |
| `metadata.embx` | attributes and documentation |
| `nested_types.embx` | aliases, nested composite use and generated C++ |

Run the corpus with:

```text
run_examples.cmd
```

The runner invokes the real EmbX CLI with `--dump-ast`. Every example must parse and pass semantic analysis.

| `next.embx` | `$next` sequential layout cursor in `at(...)` offsets |

| `midi.embx` | Standard MIDI File Type 0 structural parsing: big-endian header, track count, bounded track chunks and raw event bytes |

### MIDI real-format example

`midi.embx` describes the fixed structural layer of a Standard MIDI File without adding
MIDI-specific language mechanisms. `midi_demo.mid` is a small Type 0 file containing a
120 BPM C-major arpeggio (C4-E4-G4-C5). `play_midi.cmd` plays the fixture through the Windows MIDI sequencer API for a deterministic
audible check. The demo intentionally uses only conservative SMF events: tempo, note-on/note-off, and end-of-track.

The current EmbX layer intentionally stops at the track event byte stream. MIDI variable-
length quantities and running status belong to a later event-level format layer rather than
being introduced as special cases in the core language.
