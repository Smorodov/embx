# EmbX Test Profiles

Current audited baseline: **EmbX 0.9.84**. The preceding 0.9.83 source state was user-verified GREEN in all four profiles; this document records the unchanged profile mechanism.

The EmbX test suite contains 91 CTest tests. Test profiles change only **which registered tests are executed**; no test is disabled or removed.

## Profiles

| Command | CTest labels | Purpose |
|---|---|---|
| `build.cmd` | `CORE + ACTIVE` | Normal development gate: semantic/runtime foundation plus current generated-C++ differential work. |
| `build_extended.cmd` | `CORE + ACTIVE + EXTENDED` | Normal gate plus the broader language, course, corpus, CLI and integration coverage. |
| `build_audit.cmd` | `CORE + ACTIVE + AUDIT` | Normal gate plus focused hardening/audit tests. |
| `build_all.cmd` | all 91 tests | Full GREEN acceptance gate. |
| `clean_build.cmd` | same as `build.cmd` | Removes `build/` first, then runs the normal development gate. |
| `run_examples.cmd` | — | Runs the examples independently; it is also invoked by the build profiles. |

`build_profile.cmd` is the single shared implementation used by the profile wrappers. This keeps the build/configure procedure in one place and prevents profile-specific copies of the build logic.

## Labels

- **CORE** — parser/AST/semantic/IR/Plan invariants, reference runtime and codec foundations, symbol identity, diagnostics and runtime-parameter/conditional execution contracts.
- **ACTIVE** — current generated-C++ differential/conformance work and generated codec/callback coverage.
- **EXTENDED** — broader language features, source generation/reporting, external/corpus integration, examples and course lessons.
- **AUDIT** — focused hardening and regression tests. Some AUDIT tests intentionally overlap CORE because their protections are part of the normal safety net.

The labels are assigned in `CMakeLists.txt` after all tests are registered. `ctest -L` selects by label; the full suite remains available through `build_all.cmd`.
