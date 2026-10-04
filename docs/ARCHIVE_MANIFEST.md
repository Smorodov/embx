# EmbX Archive Manifest

Current public-preparation candidate: **EmbX 0.9.86**.

## 0.9.86 Public GitHub Preparation

- 0.9.85 remains the accepted GREEN implementation baseline (91/91 tests).
- 0.9.86 packages the public repository entry point, MIT license, contribution guide, and clean Git ignore policy.
- This milestone is not a 1.0.0 release claim.


Last accepted audited source baseline: **EmbX 0.9.84**.

Status: **PUBLIC-PREPARATION / BASED-ON-0.9.85-GREEN / POST-LANGUAGE-CLOSURE**.

The preceding source baseline, **EmbX 0.9.83**, was user-verified GREEN in all four build modes: normal development, extended, audit and full suite. The 0.9.84 change is documentation/version synchronization only; no language, AST, semantic, IR, Plan, runtime or generator behavior is changed.

Acceptance gate for the source baseline remains **91/91 CTest tests (100%)** in the target Windows/MSYS2 UCRT64 environment, with `build_all.cmd` as the full gate.

The archive is a clean source baseline. Generated build output and machine-local protocol logs are intentionally excluded.

Historical audit and milestone records remain in their original versioned form. The authoritative current audit is `docs/AUDIT_0.9.84.md`.

## Archive integrity

The release archive must contain source, tests, examples, course material, grammar, build scripts and synchronized documentation, but no `build/` directory, object files, executables, DLLs, static libraries or local build logs.

## Post-language-closure policy

The 0.9.84 source baseline remains the accepted GREEN implementation baseline. The 0.9.85 GREEN
adds only the conditional-callback generated-C++ qualification slice. No mandatory language-extension
phase remains. Remaining work is
qualification, hardening, conformance, generated-code quality/performance and release maturity.
Additional generated-language backends are community-driven; new core language constructs require
a demonstrated universal semantic gap.
