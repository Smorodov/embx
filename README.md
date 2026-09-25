# EmbX

EmbX is a declarative binary-format language with a canonical compiler pipeline:
AST → Semantic → IR → Plan → reference runtime/codecs → backends.

## Current development state

- **0.9.53 is the accepted universal array-buffer baseline.** It closes the target-neutral `ArrayDescriptor + ArrayBuffer + ArrayView` boundary, including Plan integration, dynamic dimensions, nested structures and conformance coverage.
- The canonical pipeline remains one-way and minimal: AST → Semantic → IR → Plan → reference runtime/codecs; `SymbolId` is the executable identity boundary.
- **0.9.53 acceptance:** the complete array-buffer closure is validated at **79/79 CTest tests (100%)** in the target environment.
- Generalized terminated sequences remain a language feature of the accepted 0.9.49 baseline; the course lesson adds no compiler semantics.

## Examples

The `examples/` directory is the small canonical corpus. In addition to language-feature
examples, `midi.embx` describes the fixed structural layer of a real Standard MIDI File
Type 0. `midi_demo.mid` is the corresponding four-note C-major arpeggio fixture and includes a standard Track Name text metadata event containing `EmbX`.

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
- `docs/MULTIDIMENSIONAL_ARRAY_LAYOUT.md` — normative multidimensional storage-order contract.
- `docs/MULTIDIMENSIONAL_ARRAY_VIEW.md` — target-neutral array-view boundary for final programs.
- `docs/DEVELOPMENT_HANDOFF.md` — concise continuation point for the next development stage.
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

**0.9.53 — Universal array-buffer boundary — ACCEPTED GREEN.**

The documentation freeze preceding Lesson 15 also fixes the multidimensional-array boundary: canonical shape/order remain in EmbX, while a final program may consume decoded arrays through a target-specific non-owning view rather than a language-level tensor object.

Lesson 12 adds executable teaching material for generalized terminated sequences, including a
structured sequence whose elements contain their own terminated byte payloads. Lesson 13 extends
the real MIDI fixture with a standard Track Name text metadata event. The language implementation
remains the accepted 0.9.49 contract; the course suite through Lesson 14 and the 0.9.53 array-buffer closure are accepted at 79/79 tests.
