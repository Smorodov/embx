# EmbX Execution Boundary

```text
Source
  ↓
Parser / AST
  ↓
Semantic analysis
  ↓
IR
  ↓
PlanBuilder
  ↓
validated Plan
  ↓
Runtime / Encoder / Decoder / Reflection / Codegen
```

## Compiler side

The compiler owns:

- syntax interpretation;
- SymbolId assignment;
- name resolution;
- type validation;
- alias-cycle detection;
- declaration/dependency legality;
- compile-time constant and enum evaluation;
- static layout computation;
- executable Plan validation.

## Execution side

Execution owns:

- actual byte I/O;
- runtime values;
- evaluation of already-resolved runtime expressions;
- runtime size/offset checks;
- allocation limits;
- runtime variant choice;
- callback invocation.

## Boundary invariant

After PlanBuilder returns, no component may ask “what does this declaration mean?”. It may only ask “what does this already-resolved operation do with the current runtime state?”.
