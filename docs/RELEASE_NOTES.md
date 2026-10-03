## 0.9.84 — deep architecture and documentation audit

- Audits the complete source tree, compiler/execution boundaries, C++ backend boundary, test registration and build profiles.
- Confirms **91** registered CTest tests and preserves the user-verified GREEN 0.9.83 profile state.
- Synchronizes current-status documentation to 0.9.84 while retaining historical milestone records unchanged.
- Removes stale claims that 0.9.83 was still pending GREEN verification.
- No language, AST, semantic, IR, Plan, runtime or generator semantics changed.

## 0.9.83 — test-profile build infrastructure

- Adds CTest labels for CORE, ACTIVE, EXTENDED and AUDIT coverage without disabling or removing any test.
- Adds `build_profile.cmd` as the single shared build/test implementation and thin wrappers for normal, extended, audit and full-GREEN runs.
- Keeps `clean_build.cmd` as the clean normal-development gate and keeps `run_examples.cmd` as the standalone examples runner.
- Synchronizes `VERSION` to 0.9.83 and documents the profile model in `docs/TEST_PROFILES.md`.
- No language, AST, IR, Plan, runtime, or generator semantics changed.

## 0.9.82 — differential runtime-parameter decode fixture correction

- Corrects the first runtime-parameter differential decode fixture to use the same `delta=7` parameter as the generated decode path and the encode/reference fixture.
- The previous test accidentally decoded the Reference Runtime with `delta=6`, causing `at(delta)` to overlap the sequential tail and producing a false mismatch (`0x7E` vs `0xA5`).
- No language, AST, IR, Plan, generator, or build-process changes.

## 0.9.81 — differential transform/runtime-parameter correction and floating-point coverage

- Corrects the runtime-parameter differential fixture expectations for big-endian transformed `i16` fields and absolute `at(delta)` placement without overlapping the sequential tail.
- Adds generated-C++ ↔ Reference Runtime differential coverage for `f32` and `f64` physical fields with `scale(0.5)`, including encode byte equality and decode value equality.
- No language, AST, IR, Plan, or build-process changes.
- Next differential step: continue the remaining implemented Plan combinations after floating-point transform coverage.

## 0.9.80 — transform + runtime-parameter differential qualification

- Extended the generated-C++ differential corpus so a transformed physical field is exercised together with independent runtime parameters for dynamic array size and absolute `at(...)` placement.
- The test compares Reference Runtime and generated C++ for positive and negative scale values, wire bytes, decode results, runtime-parameter values, and the resulting cursor position.
- No language or Plan semantics were changed: `scale(runtime_parameter)` remains rejected as required by the transform contract.

# EmbX 0.9.78 — Differential Transform Exactness — ACCEPTED GREEN

- Extends generated-C++ ↔ Reference Runtime differential coverage for the existing `scale` transform.
- Covers negative logical values and exact wire equality on encode/decode.
- Covers rejection of non-representable logical values and verifies failed generated encode does not publish partial output.
- No language, AST, IR, Plan, or transform semantics changed.
- Acceptance: **91/91 CTest tests (100%)** and successful `run_examples.cmd` after the local clean build.

# EmbX 0.9.72 — Differential Decode Transactional Rollback — ACCEPTED GREEN

- Fixes generated top-level decode to publish decoded state only after the complete struct succeeds.
- Failed decode callbacks therefore leave the caller-provided output object unchanged, matching Reference Runtime transactional struct decode.
- Keeps reader rollback and `consumed=0` behavior unchanged.
- No language, AST, IR, Plan, or runtime semantics changed.
- Full acceptance: **91/91 CTest tests (100%)**.

# EmbX 0.9.71 — Differential Callback Failure Qualification — ACCEPTED GREEN

- Adds generated-C++ ↔ Reference Runtime differential coverage for failed encode/decode callbacks.
- Verifies callback failure rollback without changing language semantics.
- Full acceptance remained **91/91 CTest tests (100%)**.

# EmbX 0.9.70 — Deep Architectural Audit Hardening — ACCEPTED GREEN

- Fixes copy/move ownership of generated virtual-field proxies so copied or moved objects evaluate virtual fields against the destination object.
- Makes generated virtual proxies non-assignable, preserving their owner binding.
- Adds differential regression coverage for copy, move and independent virtual-field evaluation.
- Preserves the 91-test suite; the new coverage adds assertions to the existing differential access test.

# EmbX 0.9.69 — Source Cleanup and Generated Virtual-Field Contract — ACCEPTED GREEN

- Closes the generated virtual-field accessor representation as a computed, read-only proxy rather than stored wire state.
- Synchronizes the generated codec contract so virtual fields are neither encoded nor decoded as independent fields.
- Removes stale historical audit snapshots from the active source tree and keeps one current audit record.
- Synchronizes current-status documentation with the 0.9.69 source baseline.
- Full acceptance: **91/91 CTest tests (100%)**.

# EmbX 0.9.64 — Differential Access Qualification — ACCEPTED GREEN

- Extends generated-C++ differential conformance to existing virtual fields, field aliases and scalar `scale` transforms.
- Verifies exact wire equality between generated C++ and the canonical Reference Runtime for encode.
- Verifies generated decode agreement for virtual fields and transformed logical values.
- Verifies alias write-through behavior without adding wire bytes.
- Corrects the generated encoder so virtual fields are evaluated in its working copy before later operations consume them, matching existing Plan execution order.
- No language, AST, IR or Plan semantic change was introduced.
- Full acceptance: **90/90 CTest tests (100%)**.

# EmbX 0.9.63 — Runtime-Parameter Differential Qualification — ACCEPTED GREEN

- Extends generated-C++ differential conformance to typed runtime parameters.
- Verifies runtime parameters drive both dynamic array dimensions and identifier-based `at(delta)` offsets.
- Compares generated C++ encode/decode behavior with the canonical host execution path using independent parameter values.
- Keeps parameter declarations in the existing `plan::Module.parameters` representation and emits a typed generated `RuntimeParameters` structure; no new semantic representation is introduced.
- Preserves canonical qualified SymbolId lookup in the differential test while generated C++ exposes local parameter member names.
- Removes the temporary generated-code diagnostic tooling from the accepted source baseline.
- Full acceptance: **89/89 CTest tests (100%)**.

# EmbX 0.9.62 — Terminated-Sequence Differential Qualification — ACCEPTED GREEN

- Qualifies generalized terminated sequences against generated C++ using the existing Lesson 12 corpus.
- Verifies nested terminated structured elements and outer-terminator recognition only between complete elements.
- Verifies transactional failure when the outer terminator is absent.
- Full acceptance: **88/88 CTest tests (100%)**.

# EmbX 0.9.61 — Differential Conformance Layout Expansion — ACCEPTED GREEN

- Extends the differential matrix to runtime-sized arrays and dynamic `at(offset)` layout.
- Verifies `align(4)` inside an offset region, including exact alignment padding.
- Verifies that `at(offset)` does not advance the caller's sequential cursor.
- Includes the zero-length dynamic-array boundary.
- Compares generated C++ encode/decode behavior with the canonical host execution path.
- Uses only existing language, AST, IR, Plan and runtime semantics; no language extension was introduced.
- `$next` arithmetic remains covered by the Reference Runtime layout tests and is not expanded in the generated-C++ emitter by this stage.
- Full acceptance: **87/87 CTest tests (100%)**.

# EmbX 0.9.60 — Differential Conformance Matrix — ACCEPTED GREEN

- Extends differential conformance to explicit byte order, bit fields, fixed arrays, nested structures, and variants.
- Uses one dedicated conformance corpus and one generated translation unit.
- Compares generated C++ encode/decode behavior with the canonical host execution path (Reference Runtime: Encoder/Decoder + runtime primitives).
- Corrects nested generated-structure write positioning so nested output cannot overwrite the parent writer position.
- Preserves enclosing-structure byte order for typed variant payloads.
- Synchronizes the nested generated-code contract test with the callback-aware codec boundary.
- Keeps the Reference Runtime as the canonical host execution path.
- No language, AST, IR, or Plan semantic changes were introduced.
- Full acceptance: **86/86 CTest tests (100%)**.


## C++ Generator completion plan recorded

- 0.9.78 is the accepted GREEN baseline at 91/91 CTest tests (100%).
- The remaining C++ generator work is fixed as four stages: differential completion; error/boundary hardening; generated-code/architecture audit; final conformance closure and generator freeze.
- The next differential slice starts with Transform × Runtime Parameters, followed by the remaining combinations of already implemented Plan semantics.
- No Rust/Python backend work and no language/AST/IR/Plan expansion are part of this sequence.
- The established build command files and build process remain unchanged.

## Conformance foundation retained

- 0.9.59 source corpus round-trip: **83/83 tests**.
- 0.9.59 binary conformance corpus and Reference Runtime byte-level checks are retained.
- 0.9.59 generated-backend differential conformance is retained and expanded by the 0.9.60 matrix.

## Architecture status

The canonical boundary remains:

`source → AST → Semantic → IR → Plan → Reference Runtime / Reflection / generated backends`

Reference Runtime means the existing Plan-driven Encoder/Decoder plus runtime primitives; no separate third interpreter is introduced. External format adapters remain outside the EmbX semantic core.

## 0.9.64 — Differential access conformance candidate

Adds differential generated-C++ conformance coverage for virtual fields, field
aliases, and scalar `scale` transforms. No language or Plan semantics were
added.
