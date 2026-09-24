# EmbX Execution Plan Contract

## 1. Core rule

> Compile once, execute many.

PlanBuilder resolves, validates, normalizes and computes every property that does not depend on runtime input.

## 2. Plan must contain

- declaration and field identity;
- executable types and dimensions;
- expression trees with canonical SymbolIds;
- static field sizes/offsets where known;
- static block sizes and alignments where known;
- variant structure and case expressions;
- nested operations;
- materialized constants and enum values;
- direct indices required for execution;
- runtime constraints that must be checked against actual data.

## 3. Runtime may contain

- current input/output bytes;
- current decoded/encoded values;
- dynamic lengths and offsets derived from those values;
- runtime variant selection;
- runtime bounds/allocation checks;
- callback invocation.

## 4. Runtime must not

- resolve a source name semantically;
- inspect AST or IR;
- reconstruct aliases;
- infer missing layout;
- repair SymbolIds from names;
- validate declaration order;
- maintain an obsolete compatibility semantic model.

## 5. Plan finalization gate

PlanBuilder publishes a Plan only after:

1. IR identity validation;
2. declaration/type/expression validation;
3. constant and enum materialization;
4. member lowering;
5. layout computation;
6. dependency validation;
7. layout graph validation;
8. static layout safety validation;
9. executable Plan validation.

## 6. Execution independence

The acceptance condition is that AST and IR can be destroyed immediately after Plan construction and the codecs can still execute from Plan alone.

## 7. Determinism

Identical source/configuration must produce identical executable metadata. Explicit vectors define semantic order; unordered containers are never allowed to define observable order.

## 8. Shared contract

```text
                 validated Plan
                /       |       \
           encoder    decoder   reflection
                \
               C++ backend
```

All consumers implement the same EmbX meaning.
