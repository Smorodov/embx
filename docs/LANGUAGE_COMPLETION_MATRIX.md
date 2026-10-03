# EmbX Language Completion Matrix — EmbX 0.9.84 audited baseline
> Current audited source baseline: **EmbX 0.9.84**. Historical milestone references retain their original version numbers; they are not current-status markers.

This is the current compact status view after the 0.9.57 Source Generator, 0.9.58 Format Reporter, 0.9.59 conformance foundation, the 0.9.60–0.9.64 differential closures and the 0.9.69 generated virtual-field/codec contract cleanup.
The accepted language baseline includes the executable learning course through Lesson 15. EmbX 0.9.53 additionally closes the universal multidimensional array-buffer boundary. Terminated sequences are implemented as a general bounded sequence mechanism over byte or structured elements. Yellow cells denote intentionally remaining backend/reflection qualification boundaries, not a failed release gate.

Status: `🟢 complete`, `🟡 partial`, `🔴 not complete`.

| Construct | Grammar | AST | Semantic | IR | Plan | Reference Runtime (host execution) | Reflection | C++ backend | Tests | Docs | Status |
|---|---|---|---|---|---|---|---|---|---|---|---|
| module / namespace / import | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| documentation | 🟢 | 🟢 | 🟢 | 🟢 | n/a | n/a | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| scalar types / enums / constants | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| arrays / dynamic extents | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| multidimensional arrays | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| multidimensional array physical order contract | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| remaining extents (`[*]`) | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟡 | 🟡 | 🟡 | 🟢 | 🟢 | 🟡 |
| terminated sequences (`until ... max ...`) | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| bits | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| expressions | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| struct layout / `align` / `at` / `block` / `$next` | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟡 | 🟢 | 🟢 | 🟢 | 🟡 |
| conditionals | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| variants | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| runtime parameters | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| `requires` | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| virtual fields | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟡 | 🟢 | 🟢 | 🟡 |
| field aliases | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟡 | 🟢 | 🟢 | 🟡 |
| transforms (`scale`) | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟡 | 🟢 | 🟢 | 🟡 |
| callbacks | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟡 | 🟡 | 🟢 | 🟢 | 🟡 |
| attributes | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | n/a | 🟢 | 🟡 | 🟢 | 🟢 | 🟡 |
| byte-order resolution | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| generated size properties | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟡 | 🟢 | 🟢 | 🟡 |

## C++ Generator qualification status at 0.9.84

The C++ backend has passed the current 91-test acceptance gate and the 0.9.84 architecture audit. The matrix deliberately keeps a few yellow entries where full backend/reflection qualification is narrower than the entire language surface. These are qualification boundaries, not missing core semantics:

| Stage | Scope | Status |
|---|---|---|
| 1 | Differential completion of existing Plan semantics and combinations | 🟡 next |
| 2 | Error, overflow, bounds and transactional hardening | 🟡 next |
| 3 | Generated-code quality and architecture audit | 🟡 next |
| 4 | Full conformance, documentation synchronization and C++ generator freeze | 🟡 final |

No Rust or Python backend work is part of this sequence. No language, AST, IR or Plan extension is implied by these stages.

## Backend qualification rule

A construct is green only when its applicable semantic, Plan, reference-runtime, reflection,
generated-backend and test contracts are all closed. The matrix intentionally keeps backend
partiality visible even when the reference runtime is complete.

## 0.9.53 executable learning, terminated sequences, IPv4, MIDI metadata, TLV and array-buffer closure — ACCEPTED GREEN

Lessons 1–14 are the accepted executable course and external conformance evidence. Lesson 14 adds a TLV composition example using only existing language mechanisms: expression-dependent byte arrays, structured remaining sequences and terminated sequence boundaries. The accepted language release gate before external integration was 79/79 CTest tests; the 0.9.56 source state is accepted at 80/80 after closing the external GGUF adapter conformance tests. Generalized terminated sequences remain part of the accepted language contract.

## 0.9.59–0.9.63 conformance qualification — ACCEPTED GREEN

The source corpus round-trip gate, binary conformance corpus, generated-backend differential checks and 0.9.60–0.9.64 differential matrices are accepted quality gates over the existing language semantics. The 0.9.60 matrix covers explicit byte order, bit fields, fixed arrays, nested structures and variants; 0.9.61 adds dynamic dimensions and dynamic `at(offset)` layout; 0.9.62 adds terminated-sequence differential coverage; 0.9.63 adds typed runtime-parameter differential coverage. The complete suite is accepted at **91/91 CTest tests (100%)**. These stages do not add language constructs or a second semantic model.

## Reflection closure retained from 0.9.44

PASS9–PASS14 close the principal generated-C++ operations identified by the 0.9.43 language-surface
audit. The remaining yellow cells are deliberately narrow backend/reflection qualification areas, not requests for a new semantic mechanism. The accepted 0.9.49 language baseline was locally validated at 73/73; 0.9.50 adds Lesson 12 and 0.9.51 extends Lesson 13, with the 0.9.53 source suite validated at 79/79 after the array-buffer closure; 0.9.54 introduced the external GGUF adapter boundary; 0.9.56 closes the GGML↔EmbX dimension mapping/alignment and structural-validation stage at 80/80.


## 0.9.49 acceptance

Lesson 11 — IPv4 exercises existing bit fields, big-endian integers, enums, fixed byte arrays and exact layout. Terminated sequences add one general boundary mechanism without introducing IPv4-specific semantics. The accepted 0.9.49 language suite was locally validated at 73/73 tests; the accepted 0.9.50 course adds one lesson test, 0.9.51 retains the count, and 0.9.52 adds Lesson 14; 0.9.53 closes the array-buffer boundary for the 79/79 suite; 0.9.54 accepts the external GGUF adapter at 80/80.


### Multidimensional array view boundary

The target-neutral view model is documented in `MULTIDIMENSIONAL_ARRAY_VIEW.md`. It is a future runtime/backend representation contract, not a language feature and therefore is not counted as an additional green language construct.


### Multidimensional layout contract

Multidimensional arrays are not merely marked as syntactically supported. Their physical traversal is now explicitly documented: dimension 0 is outermost and the last dimension varies fastest. This closes the previous documentation gap between `core::Type::dimensions` and the actual encoder/decoder/runtime traversal. It does not introduce a second storage-order mechanism.

## 0.9.56 external integration boundary

The GGUF stage is accepted as an external adapter only. Stage 15.3 completes adapter-owned dimension mapping, alignment validation, non-square 3-D mapping, multiple aligned tensors, nested metadata arrays and structural negative cases. It does not change the EmbX language surface, AST, Plan, runtime semantic types or universal array model.


## 0.9.63 runtime-parameter differential qualification — ACCEPTED GREEN

Typed runtime parameters are qualified in generated C++ against the canonical host execution path, including independent caller values, parameter-driven dynamic dimensions and identifier-based `at(delta)`. Acceptance is **89/89 CTest tests (100%)**.

## 0.9.62 terminated-sequence differential qualification — ACCEPTED GREEN

Nested terminated structured elements, outer-terminator boundaries and transactional missing-terminator failure are qualified against generated C++. Acceptance is **88/88 CTest tests (100%)**.

## 0.9.61 differential layout qualification — ACCEPTED GREEN

Dynamic dimensions, identifier-based `at(offset)`, alignment inside the offset region, cursor preservation and the zero-length dynamic-array boundary are qualified against generated C++ using the existing Plan. Acceptance is **87/87 CTest tests (100%)**. `$next` remains covered by the existing Reference Runtime layout tests; this stage does not change language completeness or introduce new constructs.

## 0.9.64 differential access qualification — ACCEPTED GREEN

Virtual fields, field aliases and scalar `scale` transforms receive generated-C++ differential coverage against the Reference Runtime. The matrix remains deliberately conservative: broader transform/callback combinations and conditional alias cases remain backend qualification boundaries. No language construct was added.
