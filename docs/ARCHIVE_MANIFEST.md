# Archive manifest — EmbX 0.9.53 array-buffer conformance closure

EmbX 0.9.53 is the accepted universal array-buffer baseline. It retains the accepted language/course state through Lesson 14 and closes the target-neutral multidimensional array boundary with `ArrayDescriptor`, `ArrayBuffer`, `ArrayView`, Plan integration, dynamic dimensions, nested structures and conformance coverage. The acceptance gate is the clean local build, 79/79 CTest tests (100%), and successful canonical examples.

# EmbX Source Archive Manifest

## Archive state

**EmbX 0.9.53 — universal array-buffer boundary — ACCEPTED GREEN**

This source tree contains the canonical grammar, compiler/runtime implementation, generated-C++
backend, examples, tests, executable learning material and active documentation. It contains no
generated build tree or historical project snapshot. The language implementation is the accepted
0.9.49 generalized terminated-sequence baseline and the accepted course state through Lesson 14.

## Included

- `grammar/` — canonical grammar;
- `src/` — compiler, semantic, Plan, runtime, codecs and backend implementation;
- `tests/` — current acceptance and contract suite, including the Lesson 12 and Lesson 14 regression tests;
- `examples/` — canonical source examples and the playable MIDI fixture; TLV lesson source is maintained in `course/14_tlv/` as its canonical teaching copy;
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

The accepted historical baselines remain documented in release notes, but they are not competing
active source states. The current acceptance gate is the clean local build, canonical examples and
79/79 CTest tests (100%).

## Dependency and build policy

CMake first searches for a compatible system Catch2 3 package. If one is available, the source
build uses it directly; otherwise CMake falls back to the pinned Catch2 3.15.3 source archive.
Java is discovered through CMake's normal `find_package(Java 11 REQUIRED COMPONENTS Runtime)`;
MSYS2/ANTLR discovery is derived from the active compiler or an explicit user override.

`build.cmd` writes the local build protocol to the source-root `build_protocol.txt`; it is ignored by
`.gitignore` and excluded from source archives. `clean_build.cmd` removes the entire build tree before
rebuilding.

## Current backend scope

PASS9–PASS14 cover generated C++ conditional, variant, virtual, alias, transform,
runtime-parameter expression, multidimensional-array and callback paths. Rust and Python
backends remain intentionally deferred.

Course status: Lessons 1–14 are accepted. Lesson 13 (MIDI) remains the first real-format conformance lesson. Lesson 15 (GGUF) is the next planned capstone.

- `docs/DEVELOPMENT_PLAN.md` — forward development plan, including GGUF, canonical source generation and format reporting.
- `docs/MULTIDIMENSIONAL_ARRAY_VIEW.md` — runtime/backend array-view boundary.
- `docs/DEVELOPMENT_HANDOFF.md` — operational continuation guide.


## Documentation state for the next development point

The implementation is EmbX 0.9.53. The multidimensional array physical-order contract and universal array-buffer boundary are now frozen. Lesson 15 GGUF work is the next planned integration stage; GGUF remains an external-format adapter and does not define EmbX array semantics.

The next development work is Lesson 15 GGUF. It must consume this frozen contract for GGUF/ggml dimension mapping and tensor-data access rather than introducing a second array or layout model.


## 0.9.53 closure

The universal array-buffer boundary is frozen at 0.9.53. Conformance covers 2D/3D shapes,
dynamic dimensions, named structures, nested structures, checked shape arithmetic, buffer-size
consistency, rank/bounds validation and canonical last-dimension-fastest traversal.

Acceptance: **79/79 CTest tests (100%)**.

GGUF is the next integration stage and must consume this boundary as an external-format adapter.
