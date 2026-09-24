# EmbX Backend Developer Guide

## 1. Input

A backend consumes validated `plan::Module`. It does not consume AST, parser contexts or semantic resolver state.

## 2. Semantics

The backend must not reinterpret EmbX. If Plan lacks a required semantic fact, extend the common Plan contract rather than adding a local lookup or fallback.

## 3. Target-specific work

After Plan, a backend may introduce:

- target-language types;
- ABI representation;
- ownership and containers;
- calling conventions;
- target syntax;
- optimization-specific lowering.

These are projections of Plan, not new language semantics.

## 4. Unsupported constructs

Unsupported Plan operations must fail explicitly. Ignoring an operation or silently changing its meaning is forbidden.

## 5. Conformance

The reference encoder/decoder behavior is the baseline for backend tests. New backends should add positive, negative and round-trip tests for every supported Plan construct.
