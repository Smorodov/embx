# EmbX Language Completion Matrix — 0.9.52 accepted green

This is the current compact status view after the 0.9.43 generated-C++ convergence and 0.9.44 reflection closure.
The accepted 0.9.49 language baseline includes the executable learning course through Lesson 11 and has 73 CTest registrations; 0.9.50 adds Lesson 12, 0.9.51 extends Lesson 13, and 0.9.52 adds Lesson 14. Terminated sequences are implemented as a general bounded sequence mechanism over byte or structured elements. Yellow cells denote intentionally remaining backend/reflection qualification boundaries, not a failed release gate.

Status: `🟢 complete`, `🟡 partial`, `🔴 not complete`.

| Construct | Grammar | AST | Semantic | IR | Plan | Reference runtime | Reflection | C++ backend | Tests | Docs | Status |
|---|---|---|---|---|---|---|---|---|---|---|---|
| module / namespace / import | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| documentation | 🟢 | 🟢 | 🟢 | 🟢 | n/a | n/a | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| scalar types / enums / constants | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| arrays / dynamic extents | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
| multidimensional arrays | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 | 🟢 |
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

## Backend qualification rule

A construct is green only when its applicable semantic, Plan, reference-runtime, reflection,
generated-backend and test contracts are all closed. The matrix intentionally keeps backend
partiality visible even when the reference runtime is complete.

## 0.9.52 executable learning, terminated sequences, IPv4, MIDI metadata and TLV — CANDIDATE

Lessons 1–13 remain the accepted executable course and external conformance evidence. Lesson 14 adds a TLV composition example using only existing language mechanisms: expression-dependent byte arrays, structured remaining sequences and terminated sequence boundaries. The candidate release gate is 75/75 CTest tests after clean local validation. Generalized terminated sequences remain part of the accepted language contract.

## Reflection closure retained from 0.9.44

PASS9–PASS14 close the principal generated-C++ operations identified by the 0.9.43 language-surface
audit. The remaining yellow cells are deliberately narrow backend/reflection qualification areas, not requests for a new semantic mechanism. The accepted 0.9.49 language baseline was locally validated at 73/73; 0.9.50 adds Lesson 12 and 0.9.51 extends Lesson 13, with the current accepted suite validated at 74/74; 0.9.52 adds the Lesson 14 candidate test.


## 0.9.49 acceptance

Lesson 11 — IPv4 exercises existing bit fields, big-endian integers, enums, fixed byte arrays and exact layout. Terminated sequences add one general boundary mechanism without introducing IPv4-specific semantics. The accepted 0.9.49 language suite was locally validated at 73/73 tests; the accepted 0.9.50 course adds one lesson test and is validated at 74/74.
