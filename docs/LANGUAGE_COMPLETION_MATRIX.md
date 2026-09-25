# EmbX Language Completion Matrix — 0.9.53 accepted green

This is the current compact status view after the 0.9.43 generated-C++ convergence and 0.9.44 reflection closure.
The accepted language baseline includes the executable learning course through Lesson 14. EmbX 0.9.53 additionally closes the universal multidimensional array-buffer boundary. Terminated sequences are implemented as a general bounded sequence mechanism over byte or structured elements. Yellow cells denote intentionally remaining backend/reflection qualification boundaries, not a failed release gate.

Status: `🟢 complete`, `🟡 partial`, `🔴 not complete`.

| Construct | Grammar | AST | Semantic | IR | Plan | Reference runtime | Reflection | C++ backend | Tests | Docs | Status |
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

## Backend qualification rule

A construct is green only when its applicable semantic, Plan, reference-runtime, reflection,
generated-backend and test contracts are all closed. The matrix intentionally keeps backend
partiality visible even when the reference runtime is complete.

## 0.9.53 executable learning, terminated sequences, IPv4, MIDI metadata, TLV and array-buffer closure — ACCEPTED GREEN

Lessons 1–14 are the accepted executable course and external conformance evidence. Lesson 14 adds a TLV composition example using only existing language mechanisms: expression-dependent byte arrays, structured remaining sequences and terminated sequence boundaries. The accepted release gate for the current source state is 79/79 CTest tests after clean local validation. Generalized terminated sequences remain part of the accepted language contract.

## Reflection closure retained from 0.9.44

PASS9–PASS14 close the principal generated-C++ operations identified by the 0.9.43 language-surface
audit. The remaining yellow cells are deliberately narrow backend/reflection qualification areas, not requests for a new semantic mechanism. The accepted 0.9.49 language baseline was locally validated at 73/73; 0.9.50 adds Lesson 12 and 0.9.51 extends Lesson 13, with the current 0.9.53 source suite validated at 79/79 after the array-buffer closure.


## 0.9.49 acceptance

Lesson 11 — IPv4 exercises existing bit fields, big-endian integers, enums, fixed byte arrays and exact layout. Terminated sequences add one general boundary mechanism without introducing IPv4-specific semantics. The accepted 0.9.49 language suite was locally validated at 73/73 tests; the accepted 0.9.50 course adds one lesson test, 0.9.51 retains the count, and 0.9.52 adds Lesson 14; 0.9.53 closes the array-buffer boundary for the current 79/79 suite.


### Multidimensional array view boundary

The target-neutral view model is documented in `MULTIDIMENSIONAL_ARRAY_VIEW.md`. It is a future runtime/backend representation contract, not a language feature and therefore is not counted as an additional green language construct.


### Multidimensional layout contract

Multidimensional arrays are not merely marked as syntactically supported. Their physical traversal is now explicitly documented: dimension 0 is outermost and the last dimension varies fastest. This closes the previous documentation gap between `core::Type::dimensions` and the actual encoder/decoder/runtime traversal. It does not introduce a second storage-order mechanism.
