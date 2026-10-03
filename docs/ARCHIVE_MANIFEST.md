# EmbX Archive Manifest

Current audited source baseline: **EmbX 0.9.84**.

Status: **DOCUMENTATION-AUDITED / SOURCE-GREEN-BASIS**.

The preceding source baseline, **EmbX 0.9.83**, was user-verified GREEN in all four build modes: normal development, extended, audit and full suite. The 0.9.84 change is documentation/version synchronization only; no language, AST, semantic, IR, Plan, runtime or generator behavior is changed.

Acceptance gate for the source baseline remains **91/91 CTest tests (100%)** in the target Windows/MSYS2 UCRT64 environment, with `build_all.cmd` as the full gate.

The archive is a clean source baseline. Generated build output and machine-local protocol logs are intentionally excluded.

Historical audit and milestone records remain in their original versioned form. The authoritative current audit is `docs/AUDIT_0.9.84.md`.

## Archive integrity

The release archive must contain source, tests, examples, course material, grammar, build scripts and synchronized documentation, but no `build/` directory, object files, executables, DLLs, static libraries or local build logs.
