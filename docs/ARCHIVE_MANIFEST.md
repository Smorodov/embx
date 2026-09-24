# Archive manifest — EmbX 0.9.52 TLV course update

EmbX 0.9.49 remains the accepted green language baseline with 73/73 CTest tests. EmbX 0.9.50 added the accepted Lesson 12 course coverage and retained a 74/74 suite. EmbX 0.9.51 extends Lesson 13 with standard MIDI Track Name text metadata; 0.9.52 adds the Lesson 14 TLV composition course material without changing language semantics. The 0.9.52 candidate release gate is a clean local build with 75/75 CTest tests (100%) plus a successful run of all canonical examples.

# EmbX Source Archive Manifest

## Archive state

**EmbX 0.9.52 — TLV composition lesson — CANDIDATE**

This source tree contains the canonical grammar, compiler/runtime implementation, generated-C++
backend, examples, tests, executable learning material and active documentation. It contains no
generated build tree or historical project snapshot. The language implementation is the accepted
0.9.49 generalized terminated-sequence baseline; 0.9.50 adds executable Lesson 12 coverage and
moves MIDI to Lesson 13; 0.9.51 adds standard Track Name text metadata to the Lesson 13 fixture; 0.9.52 adds Lesson 14 and its TLV fixtures.

## Included

- `grammar/` — canonical grammar;
- `src/` — compiler, semantic, Plan, runtime, codecs and backend implementation;
- `tests/` — current acceptance and contract suite, including the Lesson 12 and Lesson 14 regression tests;
- `examples/` — canonical source examples and the playable MIDI fixture;
- `course/` — executable learning material, the Lesson 12 terminated-sequence fixture, and the
  20-file MIDI conformance corpus and the Lesson 14 TLV fixtures;
  - `course/00_INTRODUCTION.md` — shared course introduction and common workflow;
- `docs/` — current vision, contracts, status and release notes;
  - `docs/TERMINATED_SEQUENCE_SPEC.md` — normative generalized terminated-sequence contract;
- build and runner scripts;
- `VERSION`.

## Excluded

- build directories;
- generated ANTLR sources;
- binaries and object files;
- generated `build/` contents;
- generated `build_protocol.txt`;
- local test artifacts;
- previous source archives;
- diagnostic/temporary duplicate project snapshots.

## Baseline and development gate

0.9.44 remains the immutable predecessor baseline with 61 CTest registrations. 0.9.45 adds the
executable learning course, the 20-case MIDI corpus checker and manual Windows playback check.
0.9.49 is the accepted 73-test language baseline with generalized structured/nested terminated
sequences. 0.9.50 adds one course test, so the acceptance gate becomes 74/74. 0.9.51 changes only the MIDI fixture, regression assertions and course documentation; 0.9.52 adds one course test, bringing the candidate gate to 75/75.

## Dependency and build policy

CMake first searches for a compatible system Catch2 3 package. If one is available, the source
build uses it directly; otherwise CMake falls back to the pinned Catch2 3.15.3 source archive.
Java is discovered through CMake's normal `find_package(Java 11 REQUIRED COMPONENTS Runtime)`;
MSYS2/ANTLR discovery is derived from the active compiler or an explicit user override.

`build.cmd` writes the local build protocol to `build\\build_protocol.txt`. `clean_build.cmd` removes
the entire build tree before rebuilding, so the protocol never becomes a source-tree artifact.

## Current backend scope

PASS9–PASS14 cover generated C++ conditional, variant, virtual, alias, transform,
runtime-parameter expression, multidimensional-array and callback paths. Rust and Python
backends remain intentionally deferred.

Course status: Lessons 1–13 are accepted; Lesson 14 (TLV composition) is candidate. Lesson 13 (MIDI) remains the first real-format conformance lesson. Lessons 15–17 remain planned.
