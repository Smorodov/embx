# Contributing to EmbX

Thank you for contributing to EmbX.

## Core principle

EmbX follows **minimality and sufficiency**: prefer one clear semantic mechanism over overlapping mechanisms, compatibility layers, or feature-count expansion.

## Before changing the language

A language change should start with a concrete, reproducible semantic gap that cannot be expressed cleanly by the existing AST and Plan model. Include a focused regression test with the change.

## Backend work

The Reference Runtime is the canonical execution path for Plan semantics. Generated backends must implement those semantics rather than redefine them. Generated C++ is the currently qualified mandatory backend; additional language backends are community-driven extensions.

## Tests

For changes affecting compiler, runtime, Plan, or generated code, run the relevant focused test first and then the complete project gate. The repository provides `build.cmd`, `build_extended.cmd`, `build_audit.cmd`, and `build_all.cmd`.

## Documentation

Keep active documentation synchronized with implementation and tests. Avoid leaving historical notes in active documents when the current contract has changed; preserve historical version records in release notes and audit documents.

## Pull requests

Please describe:

- the concrete problem being solved;
- the semantic or backend boundary affected;
- the regression tests added or updated;
- documentation changes;
- the validation performed.

Small, focused changes are preferred.
