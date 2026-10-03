# EmbX learning course

The course is executable documentation: every lesson uses a real `.embx` program,
real binary bytes, and a regression test or conformance fixture where practical.

The current course baseline is **EmbX 0.9.84**. The source baseline behind it was user-verified GREEN at 91/91 tests; 0.9.84 only synchronizes documentation and version metadata.

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
15. GGUF real-format conformance — active
16. Callback protocol — planned
17. Complete protocol — planned

Lessons 1–15 are the active teaching sequence. Lesson 15 is the GGUF capstone and has a source-preserving GGUF reference plus an EmbX-oriented restatement. Lesson 12 is the practical introduction to
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

The current course state is **0.9.84 audited baseline**, built on the accepted language baseline plus the external GGUF adapter closure and subsequent conformance/tooling stages.
0.9.50 added the generalized terminated-sequence lesson; 0.9.51 extended Lesson 13 with MIDI text metadata; 0.9.52 added the accepted TLV composition lesson; 0.9.54–0.9.56 close the GGUF external-adapter stages; 0.9.57–0.9.60 add source reconstruction, format reporting and conformance qualification without changing language semantics.
Lessons 1–15 are active; the current source acceptance gate is **91/91 CTest tests (100%)**. The 0.9.61–0.9.63 differential stages add executable qualification for layout, terminated sequences and runtime parameters without changing lesson semantics. `$next` remains taught and tested through the existing layout material. Lesson 12
covers generalized terminated sequences without adding lesson-specific compiler semantics.
Lesson 13 — MIDI remains the first real-format conformance lesson. Lesson 14 demonstrates TLV composition using only existing language mechanisms.
