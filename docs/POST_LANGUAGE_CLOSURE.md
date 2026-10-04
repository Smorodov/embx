# EmbX Post-Language-Closure Plan

## Decision

The current EmbX language surface is considered **closed for planned core expansion**.

This does not mean that the language can never evolve. It means that feature accumulation is no
longer a project goal. A new language construct must be justified by a concrete, reproducible,
universal semantic gap that the existing AST/Plan model cannot express cleanly.

## Current first implementation slice — 0.9.85 GREEN

The first concrete qualification gap selected after the 0.9.84 audit is `callback` inside an existing conditional branch. The generated backend previously rejected this combination even though both constructs were independently supported by the Plan and Reference Runtime. The candidate now emits the callback operation inside the generated conditional path and verifies `callback + conditional + scale` by differential tests in encode and decode directions.

This is a backend qualification closure, not a language extension.

## What remains

### 1. Generated-C++ qualification

Close the remaining yellow backend/reflection boundaries using the existing Plan and Reference
Runtime. Priority areas include broader callback/transform combinations, conditional alias/access
cases, attributes, generated-size/reflection boundaries and other combinations already expressible
by the language.

### 2. Robustness and hardening

Expand deterministic tests for overflow, invalid input, bounds and transactional failure. Add
property/fuzz testing where it detects classes of defects not already covered by the corpus.

### 3. Conformance corpus

Grow the source and binary corpus around combinations of existing constructs and real formats.
External-format knowledge stays in adapters and validators rather than entering the semantic core.

### 4. Generated-code quality and performance

Review generated C++ for deterministic output, unnecessary code, diagnostics and practical
performance. Optimisation must preserve the established Plan/reference conformance contract.

### 5. Runtime/API maturity

The existing non-owning multidimensional `ArrayView` and similar boundary APIs may be refined when
a concrete consumer requirement exists. These are runtime/API concerns, not new EmbX language types.

### 6. Release hardening

Maintain the four build profiles, clean-build gate, examples and full 91-test suite as permanent
acceptance gates. Synchronize documentation and prepare a 1.0 release only when the remaining
qualification and robustness evidence is sufficient.

## Community extensions

Additional generated-language backends such as Rust, Python or future targets are intentionally
**community-driven**. They are not release blockers for the EmbX core roadmap.

External-format adapters are likewise welcome when they remain outside the semantic core and do not
force format-specific compiler mechanisms.

## Rule for future language changes

Before proposing a new core construct:

1. provide a minimal reproducible example;
2. identify the exact semantic gap;
3. prove that existing constructs cannot express it;
4. define the smallest universal semantic concept that closes the gap;
5. specify it in AST/Plan terms before implementing a backend;
6. add conformance and negative tests;
7. update the normative documentation only after the rule is accepted.

The goal is **maturity and closure, not feature count**.
