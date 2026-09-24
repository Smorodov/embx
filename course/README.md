# EmbX learning course

The course is executable documentation: every lesson uses a real `.embx` program,
real binary bytes, and a regression test or conformance fixture where practical.

## Start here

Read [`00_INTRODUCTION.md`](00_INTRODUCTION.md) before the individual lessons. It is the common course reference for the EmbX compiler pipeline, available outputs, CLI usage, encode/decode model, binary-layout reasoning, error categories, validation cycle, and the boundary between shared course material and lesson-specific material.

## Current lesson corpus

1. Language and first binary format — active
2. Integers and byte order — active
3. Bit fields — active
4. Arrays and dynamic dimensions — active
5. Nested structures — active
6. Variants and conditionals — active
7. Offsets and alignment — active
8. Symbol dependencies and runtime parameters — active
9. Virtual fields and aliases — active
10. Callbacks and transforms — active
11. IPv4 — active
12. General terminated sequences — active
13. MIDI — active
14. TLV composition — active
15. GGUF real-format conformance — planned
16. Callback protocol — planned
17. Complete protocol — planned

Lessons 1–14 are the active teaching sequence. Lesson 15 is the planned GGUF capstone and has a source-preserving GGUF reference plus an EmbX-oriented restatement. Lesson 12 is the practical introduction to
general terminated sequences, including a structured element sequence terminated at element
boundaries. It uses a concrete binary fixture and the existing compiler/reference runtime.

The MIDI lesson is now Lesson 13, the first real-format conformance lesson. It contains the
original 20 supplied MIDI fixtures, a deterministic corpus checker, and a playable Type 0 demo.
The playable demo also contains a standard Track Name text metadata event (`EmbX`), so the lesson
checks practical string-like metadata inside a real binary file without introducing a MIDI-specific
string type into EmbX.

The course is developed as executable documentation: a lesson is promoted to active only
when its example is reproducible, its explanation matches the language contract, and its
practical behavior has an automated or external validation path. The course does not add
format-specific semantics to the compiler.

## Current validation

The current course state is **0.9.52 accepted green**, built on the accepted **0.9.49** language baseline.
0.9.50 added the generalized terminated-sequence lesson; 0.9.51 extended Lesson 13 with MIDI text metadata; 0.9.52 adds the accepted TLV composition lesson.
Lessons 1–14 are active and the complete suite is **75/75 CTest tests (100%)**. Lesson 12
covers generalized terminated sequences without adding lesson-specific compiler semantics.
Lesson 13 — MIDI remains the first real-format conformance lesson. Lesson 14 demonstrates TLV composition using only existing language mechanisms.
