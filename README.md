# EmbX

EmbX is a declarative binary-format language with a canonical compiler pipeline:
AST → Semantic → IR → Plan → reference runtime/codecs → backends.

## Current development state

- **0.9.47 is the accepted green executable-learning baseline through Lesson 11.** It preserves the 0.9.44 reflection contract and adds executable coverage of existing callbacks, transforms, and IPv4 layout without introducing new semantic machinery.
- The canonical pipeline remains one-way and minimal: AST → Semantic → IR → Plan → reference runtime/codecs; `SymbolId` is the executable identity boundary.
- **Baseline status:** 0.9.47 is locally validated at 72/72 CTest tests, with Lessons 1–11 active. The course lessons add executable documentation only; Lessons 10 and 11 exercise existing callback, transform, and layout contracts and do not add language semantics.

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
- `docs/TERMINATED_SEQUENCE_SPEC.md` — terminated-sequence design and 0.9.48 implementation contract.
- `docs/RELEASE_NOTES.md` — historical release record.
- `course/` — executable learning and conformance material; start with `course/00_INTRODUCTION.md`.

## Architecture

The project keeps one authoritative semantic representation per fact, canonical `SymbolId`
identity, checked 64-bit logical layout arithmetic, and Plan as the executable semantic
boundary. Format-specific behavior is not added to the language core without a complete
contract.

## Current stage

**0.9.47 — Lesson 11 IPv4 — ACCEPTED GREEN.**

0.9.47 is the accepted green baseline with 72/72 tests. It includes the executable IPv4 course lesson and its real 20-byte fixture, locally validated on Windows/MSYS2 UCRT64.

Lessons 1–11 are active. Lessons 1–11 form the executable teaching sequence from basic binary layout through callbacks and transforms: EmbX source → Plan → reference encode/decode → binary fixture → CTest. The MIDI lesson remains the first real-format conformance lesson.

The release gate is closed. Further work starts from this clean baseline; no additional semantic mechanism is implied. Rust and Python backends remain intentionally deferred.


## 0.9.48 implementation candidate

Terminated byte sequences are being implemented from the frozen design in `docs/TERMINATED_SEQUENCE_SPEC.md`. The accepted green baseline remains EmbX 0.9.47 with 72/72 tests until the new candidate is validated.
