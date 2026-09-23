# EmbX

EmbX is a declarative binary-format language with a canonical compiler pipeline:
AST → Semantic → IR → Plan → reference runtime/codecs → backends.

## Current development state

- **0.9.49 is the accepted green language baseline after the generalized terminated-sequence closure.** The 0.9.50 course update adds Lesson 12 for that existing language feature and moves MIDI to Lesson 13.
- The canonical pipeline remains one-way and minimal: AST → Semantic → IR → Plan → reference runtime/codecs; `SymbolId` is the executable identity boundary.
- **0.9.50 course-update validation gate:** the archive adds one executable lesson test to the accepted 73-test baseline. A clean local build must confirm **74/74 CTest tests (100%)** before 0.9.50 is promoted to an accepted green archive.
- Generalized terminated sequences remain a language feature of the accepted 0.9.49 baseline; the course lesson adds no compiler semantics.

## Examples

The `examples/` directory is the small canonical corpus. In addition to language-feature
examples, `midi.embx` describes the fixed structural layer of a real Standard MIDI File
Type 0. `midi_demo.mid` is the corresponding four-note C-major arpeggio fixture.

Run `run_examples.cmd` after building. On Windows, `examples/play_midi.cmd` plays the MIDI
fixture through the Windows MIDI sequencer API.

## Build

Use `build.cmd` for a clean project build and then run the canonical examples followed by
CTest. The source archive intentionally excludes generated build output and machine-local
protocol logs.

## Documentation

- `docs/VISION.md` — what EmbX is, why it exists, and the general design principles.
- `docs/LANGUAGE.md` — normative language contract.
- `docs/LANGUAGE_COMPLETION_MATRIX.md` — current implementation/status matrix.
- `docs/ARCHITECTURE.md` and `docs/ARCHITECTURE_CONTRACTS.md` — architecture and invariants.
- `docs/DEVELOPMENT_ROADMAP.md` — current roadmap and lesson plan.
- `docs/TERMINATED_SEQUENCE_SPEC.md` — normative generalized terminated-sequence contract accepted in 0.9.49.
- `docs/RELEASE_NOTES.md` — historical release record.
- `course/` — executable learning and conformance material; start with `course/00_INTRODUCTION.md`.

## Architecture

The project keeps one authoritative semantic representation per fact, canonical `SymbolId`
identity, checked 64-bit logical layout arithmetic, and Plan as the executable semantic
boundary. Format-specific behavior is not added to the language core without a complete
contract.

## Current stage

**0.9.50 — course update on the 0.9.49 accepted green language baseline.**

Lesson 12 adds executable teaching material for generalized terminated sequences, including a
structured sequence whose elements contain their own terminated byte payloads. The existing
MIDI lesson is renumbered from Lesson 12 to Lesson 13 without changing its content or MIDI
semantics. The language implementation itself remains the accepted 0.9.49 contract with 73/73
tests; the 0.9.50 course update adds one CTest lesson target and must be locally revalidated at
74/74 before release acceptance.
