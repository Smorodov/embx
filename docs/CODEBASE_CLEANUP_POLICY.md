# EmbX Codebase Cleanup Policy

Cleanup is a permanent correctness condition, not a release-time cosmetic task.

## One architecture

The tree must express one current semantic pipeline:

```text
Grammar → AST → Semantic → IR → Plan → Runtime/Backend
```

## Forbidden leftovers

Do not retain code merely because an older implementation used it. In particular, remove:

- obsolete compatibility branches;
- semantic name fallbacks;
- synthetic identity repair;
- duplicate semantic models;
- dead helpers;
- unused build variables;
- stale comments that describe removed behavior;
- documentation that contradicts the current source tree.

## Review order

1. inventory;
2. classify authoritative mechanisms;
3. add or strengthen invariant tests;
4. remove obsolete code;
5. run strict warning checks;
6. build from a clean tree;
7. run the complete test suite;
8. run canonical examples;
9. audit documentation against the final tree.

## Definition of clean

A contributor should be able to trace any semantic fact to one authoritative representation without discovering a historical fallback path.
