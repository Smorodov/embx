# EmbX 0.9.70 — Deep Audit and Source Cleanup

## Status

**Accepted green baseline**

- Version: **0.9.70**
- Acceptance gate: **91/91 CTest tests (100%)** in the target Windows/MSYS2 UCRT64 environment.
- Scope: generated virtual-field/accessor closure, generated-codec contract synchronization, source-tree cleanup and documentation synchronization.

## 1. Architectural audit

The canonical execution boundary remains:

```text
source → AST → Semantic → IR → Plan → Reference Runtime / Reflection / generated backends
```

The audit confirms the established invariants:

- Plan remains the executable semantic boundary.
- `SymbolId` remains the canonical internal identity.
- There is one semantic name-resolution path and one expression semantic model.
- Generated C++ consumes Plan semantics; it does not reconstruct language semantics from source text.
- Virtual fields remain computed values and are not wire/storage fields.
- The generated C++ representation of a virtual field is a read-only conversion proxy tied to its owning object; the codec does not encode or decode the virtual value as an independent field.
- Field aliases remain access to their physical target and do not introduce wire storage.
- Logical layout arithmetic remains checked 64-bit arithmetic.

No new language, AST, IR or Plan semantic mechanism was introduced by the 0.9.70 closure.

## 2. Generated virtual-field closure

The generated accessor path was audited after the differential access stage exposed a mismatch between the generated representation and the codec contract.

The final representation is intentionally minimal:

```text
physical field(s)
      ↓
computed virtual expression
      ↓
read-only generated proxy
```

The generated codec skips virtual operations for both decode and encode. A virtual field therefore cannot accidentally acquire independent wire storage.

The generated codec contract test was synchronized with this representation. The test no longer requires a stored `std::uint64_t doubled{}` member and explicitly verifies that the codec does not emit a `doubled` wire assignment.

## 3. Source-tree cleanup

The active source tree was checked for:

- build directories and generated compiler output;
- machine-local build logs and test protocols;
- object, executable and temporary files;
- backup/original/reject files;
- empty source artifacts;
- TODO/FIXME/HACK leftovers;
- obsolete generated-code diagnostic tooling;
- duplicate virtual-field/accessor mechanisms.

No machine-local build output or protocol log is retained in the source archive.

The historical `AUDIT_0.9.63.md` and `AUDIT_0.9.64.md` snapshots were removed from the active tree. Their milestone information remains represented by the historical release/roadmap records where appropriate. A single current audit record is now maintained as `AUDIT_0.9.70.md`.

The empty `course/midi_corpus/invalid_empty.mid` fixture is intentionally retained because it is a negative test input for an empty MIDI file.

## 4. Documentation synchronization

Current-status documents were synchronized to the 0.9.70 baseline and the 91/91 acceptance gate. Historical milestone counts remain only where they describe their corresponding completed milestone.

The active archive manifest identifies 0.9.70 as the accepted source baseline. Development handoff and current planning documents no longer describe 0.9.64 as the current baseline.

## 5. Acceptance

The 0.9.70 source baseline is accepted when the locally reproduced gate remains:

- clean source build succeeds;
- ANTLR generation succeeds;
- canonical examples pass;
- **91/91 CTest tests pass**;
- no generated build output or machine-local protocol files are present in the source archive.

No language expansion is justified by this cleanup stage.

## 6. Deep architectural hardening finding

Generated virtual fields use a proxy containing an owner pointer. The compiler-generated copy/move operations were therefore unsafe: a copied proxy could continue to reference the original object. The generated backend now emits explicit copy/move operations that copy only physical storage and rebind virtual proxies to the destination object. The proxy assignment operators are deleted so its owner binding cannot be mutated independently. A differential regression test covers copy, move and independent virtual evaluation.
