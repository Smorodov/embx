# EmbX 0.9.84 — Deep Architecture, Source and Documentation Audit

## Status

- Version: **0.9.84**
- Scope: full source-tree, architecture, test-registration, build-profile and documentation audit after the accepted 0.9.83 test-profile baseline.
- Semantic/code changes: **none**.
- 0.9.83 was user-verified GREEN for all four build modes: normal development, extended, audit and full suite.
- Registered CTest tests in the source tree: **91**.
- The archive intentionally contains source and documentation only; build output and machine-local logs are excluded.

## 1. Audit result

The audit found **no confirmed source-code defect requiring a semantic or architectural change**.

The following invariants remain intact:

1. `AST -> Semantic -> IR -> Plan` is the compiler pipeline.
2. `Plan` is the executable semantic boundary.
3. `SymbolId` is the canonical internal identity.
4. Name resolution is centralized in `core::NameResolver`.
5. `core::Expr` is the canonical executable expression representation.
6. Runtime and execution backends do not inspect AST or IR.
7. The C++ generator consumes `plan::Module`; its Plan-level alias/type/expression projection is not a second semantic resolver.
8. Reference Runtime remains the canonical execution/conformance implementation.
9. Logical layout arithmetic remains checked at 64 bits with checked host-size conversion.
10. Multidimensional array order remains defined by the single canonical `core::Type::dimensions` model.
11. External-format adapters remain outside the EmbX semantic core.
12. No second resolver, expression engine or layout engine was found.

## 2. Architecture boundary findings

### Compiler side

The parser produces AST data. Semantic analysis establishes declarations and semantic facts. IR lowering binds executable identifiers to canonical `SymbolId` values. PlanBuilder validates identity, dependencies, layout and executable structure before publishing a Plan.

### Execution side

Runtime, Encoder, Decoder and Reflection consume Plan-level facts. Runtime expression evaluation operates on already-resolved `core::Expr` objects and runtime environments; it does not perform semantic name lookup.

The string-based expression environment remains a deliberate public/runtime-facing compatibility boundary for callback-oriented APIs and tests. The SymbolId environment is the canonical executable path. This is not a second expression semantics implementation.

### Source Generator exception

`codegen/SourceGenerator` intentionally consumes AST because its purpose is canonical source reconstruction. Its header explicitly states that it performs no semantic analysis, name resolution or layout work. This is therefore not an architectural violation.

### C++ Generator

`codegen/CppGenerator` consumes Plan only. Its local helpers resolve aliases, named types, enum underlyings and Plan-bound expression references in order to project already-established Plan semantics into C++ syntax. No AST, semantic analyzer or IR object is consumed by this backend.

## 3. Source cleanliness

The source tree contains no build directory, object files, executables, DLLs, static libraries, temporary files or machine-local logs.

The only TODO/FIXME/HACK markers found in active documentation are the TODO sections in `docs/GGUF_SPECIFICATION.md`; these belong to the retained external GGUF specification and are not EmbX implementation debt.

`core/Semantic.h` is actively used by IR/Plan code and is not dead residue. The duplicated-looking `eval`/`evalSize` helpers in Encoder and Decoder are thin adapters over `runtime::evaluate`, not independent expression evaluators.

Repeated filenames such as lesson-local `README.md` or fixture names in different course directories are intentional path-local resources, not duplicate source mechanisms.

## 4. Test and build infrastructure

`CMakeLists.txt` registers exactly **91** tests.

The profile infrastructure is orthogonal to registration:

- `DEV` = CORE + ACTIVE
- `EXTENDED` = CORE + ACTIVE + EXTENDED
- `AUDIT` = CORE + ACTIVE + AUDIT
- `ALL` = all registered tests

The profile wrappers call one shared `build_profile.cmd`. The Windows command-file implementation constructs the CTest alternation internally so the `|` metacharacter is not parsed as a shell pipeline. This is the accepted fix for the previous profile-script failure.

No test is disabled or removed by a profile.

## 5. Documentation audit findings

The 0.9.83 source was GREEN, but several current-status documents still described 0.9.78, 0.9.70 or 0.9.82 as the current baseline. In particular:

- `ARCHIVE_MANIFEST.md` still said that 0.9.83 was pending local GREEN verification.
- `DEVELOPMENT_ROADMAP.md` described the 0.9.78 C++ generator baseline as the current phase anchor.
- `DEVELOPMENT_PLAN.md` retained old current-baseline wording.
- `LANGUAGE_COMPLETION_MATRIX.md` used a 0.9.78 title and stale current-phase wording.
- `FOUNDATION_CLOSURE_SPEC.md` and `RUNTIME_PARAMETERS_CONTRACT.md` contained historical current-baseline statements that were no longer current.
- `course/README.md` stopped its current lesson-status paragraph at 0.9.63 despite the active course containing Lesson 15.

These are documentation-state problems, not implementation defects.

## 6. Documentation policy after this audit

Historical milestone documents remain historical. Their original version numbers and acceptance counts are preserved.

Current-status documents must identify 0.9.84 as the documentation-audit baseline and must distinguish:

- the current source baseline;
- historical accepted milestones;
- remaining qualification boundaries;
- deferred future work.

No historical acceptance record is rewritten merely to make it look current.

## 7. Current technical position

The project is ready for a controlled next phase. The audit does **not** justify a broad refactor, a second semantic representation, a new backend abstraction, a tensor/stride model, or compatibility layers.

The remaining C++ backend yellow areas in the completion matrix are qualification boundaries where applicable coverage is narrower than the full language surface; they are not evidence of a missing core semantic mechanism.

The next feature or backend decision should therefore begin from this audited baseline rather than from another cleanup cycle.
