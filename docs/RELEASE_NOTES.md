# EmbX Release Notes

## 0.9.52 — TLV composition course lesson — ACCEPTED GREEN

- Added Lesson 14, `course/14_tlv/`, as executable teaching material for a small Type–Length–Value format.
- The lesson composes existing integer fields, expression-dependent byte arrays, structured remaining sequences and generalized terminated sequences.
- Added real TLV fixtures for normal decoding/encoding, an inner `FF FF` payload, and invalid length handling.
- Added one CTest registration, making the accepted suite 75 tests.
- Added `examples/tlv.embx` as a minimal reusable example.
- Fixed the reserved-keyword collision in the example and Lesson 14 source by naming the one-byte field `kind` rather than `type`; no grammar change is required.
- Corrected the Lesson 14 fixtures to declare big-endian record/message layout, matching the documented wire bytes; no runtime change was required.
- No TLV-specific compiler, Plan or runtime semantics were introduced.


## 0.9.51 — MIDI text metadata lesson extension — ACCEPTED GREEN

- Extended Lesson 13 with a standard MIDI Track Name meta event containing the text `EmbX`.
- Updated `midi_demo.mid` track length from 47 to 55 bytes while preserving the playable Type 0 structure.
- Extended `midi_example_test` to verify the metadata bytes and byte-for-byte round trip.
- Kept MIDI event semantics and string interpretation outside the EmbX language core.
- The accepted 0.9.50 course baseline remains 74/74; the 0.9.51 extension was validated without changing the test count, so the accepted suite remains 74/74.

## 0.9.50 — General terminated sequences course lesson — ACCEPTED GREEN

- Added Lesson 12, `course/12_terminated_sequences/`, as executable teaching material for the accepted generalized terminated-sequence language feature.
- The lesson demonstrates a structured `Entry[*] until ... max ...` sequence whose elements contain their own terminated byte payload.
- Added a concrete eight-byte binary fixture and an end-to-end lesson regression test covering Plan compilation, nested decoding, exact round-trip encoding and transactional failure when the outer terminator is missing.
- Renumbered the existing MIDI lesson from Lesson 12 to Lesson 13 without changing its content or MIDI semantics.
- Added one CTest registration; the complete 0.9.50 suite passes 74/74 tests on the clean local build.
- No compiler or runtime semantic mechanism was added by the course lesson.


# 0.9.49 — General terminated sequences — ACCEPTED GREEN

EmbX 0.9.49 generalizes the terminated-sequence operation from byte payloads to sequences of arbitrary element types, while retaining the existing `bytes until ... max ...` shorthand.

- Added `Element[*] until <HEX>... max <INT>;` for structured terminated sequences.
- Nested terminated sequences are supported.
- The outer terminator is recognized only at element boundaries, so inner terminators remain owned by nested elements.
- `max` remains the physical payload bound before the outer terminator; containing layout bounds remain authoritative.
- Decode failure remains transactional and restores the original cursor.
- Plan/LayoutBounds, Reference Runtime, Reflection and generated C++ reuse the existing semantic architecture.
- Added reference-runtime and generated-code coverage for structured and nested terminated sequences.
- Removed unused generated-C++ helpers/parameters identified during the final cleanliness audit.
- Documentation was synchronized with the accepted 0.9.49 state.
- Clean Windows/MSYS2 UCRT64 validation passed: **73/73 CTest tests (100%)**.

# 0.9.48 — Terminated byte sequences — ACCEPTED GREEN

- Added a general terminated-byte-sequence type: `bytes until <HEX>... max <INT>;`.
- Terminators are compile-time byte sequences and are not string-specific.
- The maximum is mandatory and bounds the logical payload; the physical field extent includes the terminator.
- Decode accepts the first complete terminal sequence within `maxPayload + terminatorSize`; a missing terminator fails deterministically and restores the field cursor.
- Encode appends the declared terminator and rejects payloads containing that terminal sequence; no escaping mechanism is introduced.
- Plan reuses the existing `LayoutBounds` model and classifies the type as `Bounded`.
- Reflection exposes the terminated flag, terminal bytes, maximum payload and canonical bounds.
- Generated C++ uses the existing generated codec helper path.
- Added focused terminated-sequence coverage without adding a second cursor, layout evaluator, resolver or expression representation.
- Local Windows/MSYS2 UCRT64 validation passed: **72/72 CTest tests (100%)**.

# 0.9.47 — ACCEPTED GREEN

- Added Lesson 11 — IPv4 as executable course documentation.
- Added a concrete 20-byte IPv4 header fixture and end-to-end codec test.
- Validates exact layout, bit fields, big-endian integers, enum-backed protocol, fixed byte arrays and independent checksum property.
- No IPv4-specific language or runtime semantics were introduced.
- Local Windows/MSYS2 UCRT64 validation passed: **72/72 CTest tests (100%)**.

## 0.9.46 — Lesson 10 callbacks and transforms — ACCEPTED GREEN

- Lesson 10 is active and locally validated as executable documentation for existing callbacks and transforms.
- Decode and encode callbacks are demonstrated in separate structs because the reference runtime rejects a callback whose direction is opposite to the active codec path.
- `scale(0.1)` is demonstrated as a logical/physical transform without changing the `i16` physical layout.
- The complete local CTest suite passes **71/71 tests (100%)**.
- No new compiler, Plan, or runtime semantic mechanism was introduced for the lesson.

## 0.9.45 — Executable course Lessons 1–9 — LOCALLY VALIDATED

- Lessons 1–9 are active executable documentation covering the first binary format, integers/byte order, bit fields, arrays/dynamic dimensions, nested structures, variants/conditionals, offsets/alignment, symbol dependencies/runtime parameters, and virtual fields/aliases.
- The complete course sequence passes **70/70 CTest tests (100%)** in the local Windows/MSYS2 UCRT64 validation.
- Lesson 9 verifies that virtual fields and aliases participate in later expressions without duplicate wire storage and without introducing new semantic/runtime machinery.
- Documentation was synchronized with the actual active lesson set and current test count.
- No language semantic change was introduced by the course material.

## 0.9.45 — Lesson 4 candidate: arrays and dynamic dimensions (historical development entry)

- Added Lesson 4 with fixed arrays and runtime-sized arrays using existing dimension semantics.
- Added two end-to-end regression cases covering exact fixed layout and runtime-dependent layout.
- No new semantic or runtime mechanism was introduced.
- At that development point, the candidate registered 65 CTest tests and local Windows/MSYS2 UCRT64 validation was pending.

## 0.9.45 — Executable learning & MIDI conformance — ACCEPTED GREEN

- Added the first `course/` executable-learning structure.
- Added Lesson 1 as the first complete end-to-end executable teaching path, including the exact seven-byte binary fixture and regression test.
- Added the supplied 20-file MIDI corpus as practical conformance material.
- Added an independent corpus checker outside the EmbX semantic core.
- Simplified `midi_demo.mid` to conservative Type 0 events: tempo, note-on/note-off and end-of-track.
- Replaced file-association playback with a Windows MCI sequencer helper and explicit error reporting.
- Kept `midi.embx` structural semantics unchanged.
- No new resolver, expression IR, runtime evaluator or layout algorithm was introduced.
- The pre-Lesson-4 0.9.45 baseline passed with **64/64 CTest tests (100%)**, canonical examples, the 20-case MIDI corpus and the manual MIDI playback check.

## 0.9.44 — Reflection and contract closure — ACCEPTED GREEN

Builds directly on the accepted 0.9.43 C++ backend convergence baseline.

- Reflection exposes struct layout bounds and `Exact` / `Bounded` / `Unbounded` classification from existing Plan layout facts.
- Reflection preserves field and alias `SymbolId` identity and alias target identity.
- Reflection marks `[ * ]` field extents as unbounded without introducing a second size evaluator.
- Existing virtual, callback and transform metadata remain on the existing Plan-to-reflection path.
- Added contract coverage for layout bounds, remaining extents and symbol identity.
- No new resolver, expression IR, runtime evaluator or layout algorithm was introduced.

0.9.44 is now the immutable accepted baseline. The local Windows/MSYS2 UCRT64 gate passed with 61/61 CTest tests.

## 0.9.43 — PASS9–PASS14 — C++ backend convergence — ACCEPTED GREEN

- PASS9: generated C++ conditional execution, including nested conditional composition.
- PASS10: generated C++ variant representation, discriminator selection and payload consistency checks.
- PASS11: generated C++ virtual fields, field aliases and `scale` transforms.
- PASS12: generated C++ runtime-parameter propagation and canonical `Plan::Expr` emission.
- PASS13: generated C++ multidimensional fixed/dynamic array representation and recursive codecs.
- PASS14: generated C++ callback registry with explicit encode/decode direction, scalar callback arguments and transactional failure handling.
- No second resolver, expression representation, runtime evaluator or layout algorithm was introduced.
- Final acceptance: clean Windows/MSYS2 UCRT64 build, canonical examples and **61/61 CTest tests (100%)**.

## 0.9.43 — PASS10

- Added generated C++ representation and codec handling for the existing Plan `variant` operation.
- Corrected variant codec regression coverage to distinguish default-branch mismatch from the no-default no-match path.
- Preserved discriminator semantics from the canonical Plan; no second semantic resolver was introduced.
- Added generated-code regression coverage for typed cases, inline cases, default selection and discriminator/payload consistency.


## 0.9.41 — Lexical Name Resolution / Shadowing — ACCEPTED

- Removed premature qualification of unqualified value identifiers from `ParserDriver`.
- Kept explicit qualified-name/import-alias canonicalization in the parser.
- Exposed current-module declarations by local spelling in the module `Scope`, allowing `NameResolver` to apply local-over-module shadowing.
- Preserved canonical qualified names and SymbolId-based Plan/runtime execution.
- Added end-to-end contracts for field shadowing of constants/parameters and qualified-name bypass.
- Acceptance: local Windows/MSYS2 UCRT64 build, canonical examples and **60/60 CTest tests (100%)**.

## 0.9.40 — Expression Typing Closure — ACCEPTED

The expression-typing audit identified a closure gap at the executable Plan boundary. The corrective implementation resolves declaration-derived expression types by canonical `SymbolId`, validates logical/comparison/arithmetic/unary operators consistently, and treats `true`/`false` as canonical boolean literals. Attribute parsing explicitly preserves boolean attribute values after those spellings became reserved literal tokens. The cleaned 0.9.40 acceptance gate passed **60/60 meaningful CTest tests (100%)**. No second expression IR or resolver was introduced.

## 0.9.40 — MIDI Real-Format Example — ACCEPTED

- Added a real Standard MIDI File Type 0 fixture and an EmbX schema for its fixed structural layer.
- Added a regression test that compiles `examples/midi.embx` and decodes `examples/midi_demo.mid` through the reference decoder.
- Added a simple Windows launcher for opening the MIDI fixture in the registered MIDI application.
- Kept MIDI variable-length quantities and running status outside the language core; the track event stream remains raw bytes for a future format-level layer.
- Fixed reference Encoder/Decoder execution of runtime-sized arrays of named values; dynamic dimensions are evaluated from the executable environment instead of being rejected as byte/string-only shapes.
- Closed the reference codec boundary for one-element named arrays: an array dimension remains an array even when its evaluated count is `1`.
- Hardened the reference bit-container path for the full 1..64-bit width matrix, including the width-1 boundary.
- Final acceptance: clean local build, canonical examples and **60/60 CTest tests (100%)**.

## 0.9.39 — Variant Closure — ACCEPTED

- Materialized compile-time variant case tags in executable Plan.
- Removed runtime evaluation and layout dependency edges for case tags.
- Added checked discriminator/tag domain validation and semantic duplicate-tag detection.
- Added reference runtime and reflection coverage for explicit/default dispatch.
- Added Variant Closure contract and regression tests.
- Generated C++ Variant support remains explicitly partial.

Acceptance: clean local build and **59/59 CTest tests (100%)**.

## 0.9.38 — `requires` — ACCEPTED

Adds canonical struct-level `requires expression;` constraints using the existing `core::Expr` and Plan pipeline. Runtime enforcement is symmetric for encode/decode; reflection exposes requirement expressions; generated C++ reproduces the supported requirement subset. Clean local acceptance passed with 58/58 CTest tests (100%).


## 0.9.37 — Byte-Order Closure — ACCEPTED

- Closed byte-order inheritance with one canonical rule: module default → struct override → field override.
- Standardized `little`, `big` and explicit `native` semantics.
- Preserved resolved `native` in Plan and Reflection; generated C++ maps it to host byte order.
- Encoder and Decoder consume the resolved Plan byte order without a second inheritance resolver.
- Added regression coverage for inheritance, overrides, native preservation, reflection and generated C++ handling.
- Synchronized the foundation audit, completion matrix, roadmap and archive manifest.
- Acceptance: clean local build, canonical examples and **57/57 CTest tests (100%)**.

## 0.9.36 — Attribute Closure — ACCEPTED

- canonical attribute definitions are carried into IR and Plan;
- attribute uses no longer duplicate declared-type state;
- semantic validation distinguishes marker, integer, float and string values;
- reflection exposes attribute definitions and uses;
- no runtime attribute evaluator or second attribute-resolution mechanism is introduced.

Acceptance: clean local build, examples and **56/56 CTest tests passed (100%)**.

## 0.9.35 — Generated Size Properties — ACCEPTED

Adds canonical generated size properties derived exclusively from the existing `LayoutBounds` model:

- `$size_in_bytes` for exact layouts;
- `$min_size_in_bytes` for layouts with a defined minimum;
- `$max_size_in_bytes` for layouts with a finite maximum.

The properties use canonical builtin `SymbolId` identities and are materialized in the current struct execution environment. They are not declarations and do not create `LayoutGraph` dependency edges. No second size/layout algorithm was introduced.

Acceptance: clean local build, examples, and **56/56 CTest tests passed (100%)**.

## 0.9.35

- Added generated size properties `$size_in_bytes`, `$min_size_in_bytes` and `$max_size_in_bytes`, derived from canonical Plan `LayoutBounds`.
- Added checked `uint64_t` size-property semantics and execution-time exposure to canonical expressions.
- Added regression coverage for exact generated size properties and their canonical SymbolIds.


### Candidate correction
Generated size-property builtins are excluded from LayoutGraph dependency edges because they are Plan-derived values, not semantic declarations.

## 0.9.34 — Transforms — ACCEPTED

Adds the first transform implementation: physical-field `scale(factor)`, with checked compile-time factor validation, reference Encoder/Decoder execution, zero layout extent, alias reuse of the physical target transform, reflection metadata, and numeric hardening against silent narrowing and wide-integer precision loss. The 0.9.34 clean acceptance gate passed.

## 0.9.33 — Write-through Field Aliases — ACCEPTED (historical)

- Field aliases can be supplied as encoder input names for their earlier physical targets.
- The target field may be omitted when its alias input is supplied; the existing target SymbolId is seeded before sequential encoding.
- Supplying both target and alias is allowed only when their scalar values agree.
- Alias inputs targeting read-only virtual fields are rejected as non-writable.
- Alias wire/layout semantics remain unchanged: aliases consume no bytes and retain independent SymbolId identity.
- Added positive and negative encoder coverage for writable aliases, conflicting dual inputs and virtual targets.
- Transforms were intentionally deferred at the 0.9.33 boundary; they are accepted in 0.9.34.
- Acceptance: the 0.9.33 local acceptance gate passed with the canonical examples and 55/55 CTest tests.

## 0.9.32 — Field Aliases — ACCEPTED SOURCE BASELINE (historical)

- Added `alias name = field;` as a minimal read-only struct field alias.
- Field aliases have their own canonical `SymbolId` and lexical identity.
- Aliases consume no wire bytes and are available to later expressions/layout dimensions.
- Reference Encoder and Decoder materialize alias values without requiring callers to provide them.
- Reflection exposes aliases as a distinct member kind.
- Forward references and non-field alias targets are rejected by Plan dependency validation.
- Write-through aliases were intentionally deferred at the 0.9.32 boundary and accepted in 0.9.33; transforms were accepted in 0.9.34.
- Added parser, IR, Plan, layout-graph, codec and reflection regression coverage.
- Acceptance was completed as the 0.9.32 source baseline before the 0.9.33 extension.

## 0.9.31 — Virtual Fields — ACCEPTED PROGRESSION (historical)

- Added `let name = expression;` as a struct virtual-field construction.
- Virtual fields have canonical `SymbolId` identity through Semantic → IR → Plan.
- Virtual fields consume no wire bytes and participate in executable dependency validation.
- Reference Encoder and Decoder evaluate virtual fields; Decoder exposes the derived value.
- Virtual fields can feed later layout expressions.
- Field aliases remain deferred in 0.9.31; write-through alias inputs and transforms are addressed by later releases.
- Duplicate field diagnostics are emitted at the Semantic stage as `duplicate field: <name>`, matching the semantic-negative contract.
- CMake now prefers a compatible system Catch2 3 installation and falls back to the pinned 3.15.3 source only when none is available.
- Acceptance was completed before the subsequent field-alias stages; the baseline was validated as part of the progression to 0.9.32.

## 0.9.30 — `$next` Layout Semantics — ACCEPTED (historical)

- Implemented `$next` as a canonical built-in layout value for `at(...)` offset expressions.
- `$next` denotes the end of the immediately preceding sequential layout operation and supports arithmetic such as `$next + 2`.
- Static Plan layout materializes `$next` when the sequential cursor is statically known; runtime codecs provide the current cursor for dynamic cases.
- Added parser, semantic-negative, Plan and encoder/decoder regression coverage.


## 0.9.30 — Conditional Runtime Execution

- Implemented reference encoder execution of `if` / `else` Plan operations.
- Implemented reference decoder execution of `if` / `else` Plan operations.
- Conditions are evaluated through the existing runtime expression evaluator and therefore use the same field and runtime-parameter environment as other executable expressions.
- Selected branch members populate the normal value/environment state; the non-selected branch is not accessed or consumed.
- Added codec regression coverage for both branches and for conditions driven by runtime parameters.

## 0.9.28 — `$next` Layout Semantics and Conditional-Field Foundation

**Acceptance:** 51/51 tests passed; canonical examples passed. This version is the current canonical source baseline.


- Added minimal `if (expr) { ... }` / optional `else { ... }` member syntax.
- Conditions use canonical `core::Expr` and require boolean type.
- Conditional branches have independent scopes; branch-local names do not leak into the enclosing scope.
- Added executable Plan `Conditional` operations and dependency validation for condition inputs.
- Reused canonical layout bounds for conditional alternatives without introducing a second size model.
- Added semantic, IR, Plan, dependency-graph and bounds regression coverage.


## 0.9.26 — Layout Composition Closure

- Unified static/dynamic layout-bound composition for sequential members, alignment, variants and absolute `at(...)` regions.
- `at(offset)` now contributes its nested extent from the absolute offset without being incorrectly accumulated as a sequential size.
- Nested `at(...)` regions remain absolute/root-relative, matching executable encoder/decoder semantics.
- Alignment inside an `at(...)` region is evaluated from that region's actual absolute cursor.
- Added regression coverage for nested absolute placement, alignment, dynamic arrays inside `at(...)` and checked extent overflow.
- No new language syntax or semantic concepts are introduced.

## 0.9.25 — Layout Bounds Dynamic-Dimension Closure

- Fixed canonical `layoutBounds` classification for `Dynamic` dimensions.
- Dynamic dimensions are now explicitly treated as runtime-sized: minimum size is 0 and no finite maximum is claimed.
- The existing exact/bounded/unbounded model and checked `uint64_t` arithmetic are unchanged.
- This is a correctness closure of the 0.9.24 layout-bounds implementation; no language syntax or semantic model is added.

## 0.9.24 — Layout Bounds Closure

- Added the canonical Plan `LayoutBounds` model with checked `minSize` and optional finite `maxSize`.
- Added explicit `Exact`, `Bounded` and `Unbounded` classification.
- Derived bounds from canonical type dimensions, aliases, nested structs, blocks, variants, alignment and absolute `at(...)` extents.
- Kept checked `uint64_t` arithmetic for logical layout quantities and added regression coverage for exact, bounded and unbounded cases.
- Deferred `$next` until the bounds model is stable, avoiding a second temporary layout semantics.


## 0.9.23 — Layout Type-Shape Closure

- Removed the legacy duplicate AST field-length storage. Field length modifiers now exist only in `FieldModifier`; AST → IR lowering canonicalizes them into `core::Type::dimensions`.
- Removed parser, semantic and IR dependencies on the former `Field::length` compatibility slot.
- Kept source syntax in AST while maintaining one semantic type-shape representation after lowering.
- Updated regression tests that construct field length modifiers directly.
- Updated `build.cmd`: canonical examples now run before CTest, and the script pauses at the end (including failure exits) for convenient Windows console inspection.

## 0.9.21 — Expression Semantics Baseline

- Removed the duplicate AST expression representation; `ast::Expr` is now an alias of canonical `core::Expr`.
- Added canonical expression result typing: signed integer, unsigned integer, floating, boolean, string, unknown and invalid.
- Added explicit operator type rules for arithmetic, comparison and logical operators.
- Enforced boolean operands for `&&` and `||`; runtime short-circuit behavior remains unchanged.
- Preserved checked integer arithmetic and made `%` explicitly integer-only.
- Added focused expression-type regression coverage.

## 0.9.29 — Runtime Parameters

Runtime parameters are introduced as typed module-level execution inputs. The current implementation closes declaration and identity through Grammar → AST → Semantic → IR → Plan, exposes parameter metadata through Reflection, and supplies execution-local `SymbolId → Value` environments to the runtime Encoder and Decoder. Parameter-dependent expressions therefore use the existing evaluator and checked layout machinery. Generated C++ parameter API generation remains explicitly deferred; the runtime-parameter contract itself is accepted. The 0.9.29 clean local acceptance gate passed with 52/52 CTest tests.

### 0.9.42 — AST → Plan → Reference Interpreter Audit — ACCEPTED GREEN

- Closed PASS1–PASS8 of the executable semantic-path audit.
- Hardened runtime installation of module constants and enum items by canonical SymbolId.
- Closed transactional `at(...)` rollback, multidimensional-array shape, runtime-parameter symmetry, conditional/variant execution, layout cursor semantics, size/transform/callback boundaries, and malformed Plan identity.
- Removed the remaining machine-local JDK path from CMake; Java discovery is now environment/toolchain based.
- Final acceptance: **60/60 CTest tests (100%)**.
- No duplicate resolver, expression IR, layout evaluator or runtime semantic mechanism was introduced.

### 0.9.43 PASS13 — Multidimensional C++ generator closure

The C++ backend now recursively represents and encodes/decodes canonical multidimensional
array extents, while preserving the established `bytes[*]` / `string[*]` remaining-input form.
