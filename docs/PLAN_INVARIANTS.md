# EmbX Plan Invariants

A Plan returned by PlanBuilder must satisfy every invariant below.

1. Every module declaration that participates in semantics has a valid SymbolId.
2. Every executable field and bit-field has a valid field SymbolId.
3. Every named type points to a known type declaration.
4. Every executable identifier expression carries a known SymbolId.
5. `bytes` and `string` are terminal built-in types.
6. `[*]` is represented by `Dimension::Remaining` and has no expression.
7. Statically evaluable execution dimensions are canonicalized to literals.
8. Genuinely runtime-dependent expressions remain explicit.
9. Nested operations contain all execution data they require.
10. Static offsets, sizes and alignments are materialized whenever their operands are compile-time known.
11. Constant and enum values required by execution are materialized.
12. Direct SymbolId-to-Plan indices are valid.
13. No execution decision depends on IR ordering metadata.
14. No execution path depends on pointer identity.
15. Unordered containers do not define semantic order.
16. A failed validation prevents Plan publication.
17. Plan remains executable after AST and IR destruction.
18. Runtime does not inspect AST, IR or perform semantic name resolution.
19. Public string lookup is limited to explicit API boundaries.
20. `Expr::text` cannot repair unresolved execution identity.
21. Encoder and decoder consume the same Plan semantics.
22. Logical layout arithmetic is checked at 64 bits and host conversion is checked separately.
23. Runtime resource limits do not silently change valid language meaning; they fail explicitly.
24. Identical source/configuration produces deterministic executable metadata.
