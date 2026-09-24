# EmbX 0.9.29 — Runtime Parameters Contract

**Status:** Accepted in 0.9.46 and retained in the 0.9.48 green baseline.
Status: Accepted implementation contract
Baseline: EmbX 0.9.29 accepted runtime-parameter baseline (derived from the EmbX 0.9.28 canonical baseline)

## 1. Goal

Runtime parameters are typed execution inputs supplied by the caller of a compiled Plan. They allow expressions whose values are not known at compile time to depend on explicit external inputs without exposing AST, semantic, parser, or IR objects to runtime.

The accepted runtime-parameter implementation closes Grammar → AST → Semantic → IR → Plan → Runtime → Encoder/Decoder → Reflection → Tests → Documentation. Generated C++ runtime-parameter propagation is part of the accepted backend contract; the generated API consumes the same canonical Plan parameter facts.

## 2. Non-goals

Runtime parameters are not:

- module constants;
- fields read from the encoded object;
- callback-local arguments;
- mutable global variables;
- strings resolved dynamically by name;
- a second expression or layout system.

The existing canonical `core::Expr`, `SymbolId`, `Plan`, `LayoutBounds`, and runtime evaluator remain authoritative.

## 3. Source syntax

Introduce an explicit parameter declaration at module scope:

```embx
param limit: u32;
param mode: u8;
```

The declaration name is a module-level symbol and therefore participates in the canonical module-wide symbol table.

Parameters have no initializer. A parameter value is supplied by the execution caller.

The initial implementation supports scalar primitive parameter types only:

- u8, u16, u32, u64
- i8, i16, i32, i64
- f32, f64
- boolean is reserved for a later language extension unless the existing source type system is explicitly extended to expose it as a declaration type.

A parameter may not be an array, `bytes[*]`, `string[*]`, struct, enum item, or callback.

## 4. Symbol identity

Each parameter receives one canonical `SymbolId` from the module `SymbolTable`.

Parameter symbols are module/global symbols, not scoped field symbols.

All executable expressions referencing a parameter carry its `SymbolId` in `Expr::reference.id`.

Runtime must resolve parameter values by `SymbolId`, never by string lookup.

`Expr::text` remains source/diagnostic presentation only.

## 5. Scope and availability

Parameters are available throughout the module's executable struct plans, subject to normal declaration/use validation.

A parameter is available before and after fields, conditionals, variants, blocks and `at(...)` operations.

A parameter does not become a field and does not enter the sequential field environment.

Field availability rules remain unchanged: a field is available only after its sequential declaration/operation has executed.

Branch-local fields remain branch-local.

A parameter may be referenced by:

- field dimensions;
- block size expressions;
- `at(...)` offsets;
- `align(...)` expressions;
- conditional expressions;
- variant discriminators and tags where existing dependency rules permit them;
- callback arguments;
- other executable expression positions explicitly accepted by semantic rules.

A parameter may not be used as `$next`; `$next` remains a distinct built-in layout value.

## 6. Type semantics

Parameter declarations have a declared primitive type.

At runtime the supplied `Value` must be compatible with that declared type.

No implicit narrowing conversion is permitted.

Signed and unsigned integer parameters remain distinct. Negative values are rejected wherever a layout quantity is required by `evaluateSize`.

Floating parameters may participate only in expression contexts already legal for floating expressions. They cannot silently become layout sizes or offsets.

The parameter type must be represented in the executable Plan so runtime validation does not need AST or semantic structures.

## 7. Runtime environment

Replace the current implicit distinction between internal expression values and caller inputs with an explicit execution environment concept.

Conceptually:

```text
RuntimeEnvironment
  parameter values: SymbolId → Value
  field values:     SymbolId → Value
  built-ins:        SymbolId → Value
```

The existing `SymbolEnvironment` may remain the concrete implementation if it can enforce the required separation and validation.

`BuiltinNextSymbolId` is not a caller parameter. It is injected temporarily by the `at(...)` execution path exactly as in 0.9.28.

The runtime environment must be owned by a single codec execution and must not be global or shared implicitly between concurrent executions.

## 8. API contract

Encoder and decoder receive runtime parameters through an explicit typed execution input.

Recommended public shape:

```cpp
struct ParameterValue {
    core::SymbolId symbol;
    Value value;
};

using ParameterEnvironment = std::unordered_map<core::SymbolId, Value>;
```

The public codec API should accept a parameter environment independently of callbacks.

The implementation may internally merge parameters with the per-execution field environment, but the distinction must remain enforceable so a field cannot overwrite a parameter binding.

Duplicate parameter assignments are an error.

Missing required parameters are an error before execution of a plan that can reference them.

Extra parameter values are an error by default; accepting unused values would hide caller/schema mismatches.

## 9. Plan representation

`plan::Module` contains the executable parameter declarations.

Each declaration contains at minimum:

```cpp
struct Parameter {
    core::SymbolId symbol;
    std::string name;
    Type type;
};
```

The Plan must contain sufficient metadata to validate supplied values without consulting AST/IR/semantic objects.

No expression should retain an unresolved parameter name as executable identity.

## 10. Static Plan construction

Parameters are intentionally unresolved at compile-time value evaluation.

When PlanBuilder attempts constant materialization of an expression containing a parameter, the expression remains dynamic rather than becoming an error.

This is different from an unknown symbol: a declared parameter is a valid runtime dependency.

Therefore:

```text
undeclared identifier → semantic error
```

but:

```text
declared parameter → valid dynamic expression
```

Static bounds must remain conservative.

If a parameter controls a size/offset and no finite bound can be derived from the parameter declaration, the resulting layout classification is `Unbounded`.

A future parameter range annotation may provide finite bounds, but that is outside 0.9.29 unless explicitly specified and implemented as part of the same contract.

## 11. Layout interaction

Parameter-dependent expressions use the existing checked `uint64_t` logical layout model.

Examples:

```embx
param payload_size: u32;

struct Packet {
    length: u32;
    block payload[payload_size] {
        bytes[*];
    }
}
```

and:

```embx
param header_offset: u32;

struct Packet {
    id: u8;
    at(header_offset) {
        value: u32;
    }
}
```

Runtime must check:

1. expression evaluation succeeds;
2. signed values are non-negative where a layout quantity is required;
3. `uint64_t` arithmetic does not overflow;
4. conversion from logical `uint64_t` to host `size_t` succeeds;
5. configured resource limits are respected;
6. active block/reader/writer limits are not violated.

## 12. `$next` interaction

`$next` and parameters are orthogonal.

For:

```embx
param delta: u32;

struct S {
    a: u8;
    at($next + delta) {
        b: u8;
    }
}
```

runtime evaluation receives both bindings:

```text
BuiltinNextSymbolId → current sequential cursor
parameter SymbolId   → caller-supplied delta
```

Neither binding is represented as a string lookup.

Static materialization is possible only when every operand required by the expression is compile-time known. A runtime parameter therefore normally keeps the `at` offset dynamic.

## 13. Encoder/decoder symmetry

The same parameter environment is supplied to both directions.

Given identical Plan, parameter values and logical input value, encoder and decoder must interpret parameter-dependent expressions identically.

The following must use the same evaluated value in both directions:

- dynamic field dimensions;
- block sizes;
- `at` offsets;
- alignment;
- conditional predicates;
- variant discriminators;
- callback arguments.

No encoder-only or decoder-only parameter semantics are permitted.

## 14. Conditional fields

Parameters may be used in conditional predicates:

```embx
param version: u8;

struct Packet {
    if (version == 2) {
        value: u32;
    }
}
```

The parameter is always available; branch-local fields remain unavailable outside the branch.

Static layout bounds follow the existing conditional `LayoutBounds` rules.

Runtime evaluates the predicate using the supplied parameter environment and then executes exactly one branch.

## 15. Callback interaction

Callback arguments may reference parameters.

Callbacks receive parameter values through the existing runtime environment mechanism rather than through a second parameter API.

The callback interface must not gain access to AST, IR, Plan internals, or unresolved names.

The same parameter values must be visible in encode and decode callbacks.

## 16. Validation phases

### Grammar

Add `paramDecl` to module items.

### AST

Add a parameter declaration node containing name and `TypeRef`.

### Semantic

Validate:

- unique declaration name;
- legal parameter type;
- canonical SymbolId assignment;
- expression references resolve to parameter symbols;
- illegal parameter types are rejected;
- existing availability rules remain intact.

### IR

Carry parameter declaration and canonical symbol identity.

### Plan

Materialize parameter declarations and their executable types.

### Runtime

Validate and install caller-supplied parameter values into the execution environment.

### Encoder / Decoder

Use the same validated environment for every expression evaluation.

### Reflection

Expose parameter declarations and their types.

### C++ backend

Generated codec entry points expose the required parameter inputs without exposing compiler internals.

## 17. Diagnostics

Diagnostics must distinguish:

- unknown parameter name;
- missing required parameter;
- duplicate parameter value;
- extra parameter value;
- parameter type mismatch;
- negative parameter used as a layout quantity;
- parameter-dependent expression overflow;
- parameter-dependent host-size overflow;
- parameter-dependent block/array exceeding configured limits.

Diagnostics should use source names and source locations where available, while executable lookup remains SymbolId-based.

## 18. Reflection

Reflection exposes, at minimum:

- parameter name;
- SymbolId;
- declared type.

Reflection must not expose internal AST/IR objects.

Parameter order in reflection must be deterministic and match source declaration order.

## 19. Concurrency and lifetime

Parameter environments are execution-local.

No parameter value may leak between encoder/decoder instances or between sequential/concurrent executions.

The compiler and Plan remain immutable during codec execution.

## 20. Security and robustness

Parameters are untrusted runtime inputs.

All parameter-driven layout quantities must pass the same checked arithmetic, host-size conversion and configured resource-limit checks as other runtime expressions.

A malicious parameter must not permit:

- integer wraparound;
- negative-to-unsigned reinterpretation;
- out-of-range host allocation;
- reader/writer limit escape;
- uncontrolled recursion;
- stale environment reuse.

## 21. Tests required for acceptance

At minimum:

1. parameter declaration parses;
2. parameter receives canonical SymbolId;
3. duplicate declaration is rejected;
4. illegal parameter type is rejected;
5. parameter reference resolves in expressions;
6. undeclared parameter reference is rejected;
7. missing runtime parameter is rejected;
8. extra runtime parameter is rejected;
9. duplicate runtime parameter is rejected;
10. correct integer parameter evaluates correctly;
11. signed/unsigned type mismatch is rejected;
12. floating parameter cannot become a layout quantity;
13. dynamic field dimension uses parameter;
14. dynamic block size uses parameter;
15. `at(parameter)` works;
16. `$next + parameter` works;
17. parameter in conditional predicate works;
18. parameter in variant discriminator works;
19. parameter in callback argument works;
20. encoder and decoder produce symmetric behavior;
21. parameter-dependent layout bounds remain conservative;
22. parameter values do not leak between executions;
23. host-size overflow is rejected;
24. configured allocation/element limits are enforced;
25. reflection exposes deterministic parameter metadata;
26. generated C++ API carries parameters correctly; *(deferred from the 0.9.29 contract; not required for runtime-parameter acceptance)*
27. full existing 0.9.28 suite remains green; the accepted 0.9.29 suite contains 52 passing CTest tests.

## 22. Acceptance gate

The 0.9.29 runtime-parameter contract is accepted. The recorded acceptance gate is:

- clean build from an empty `build` directory succeeds;
- ANTLR generation succeeds;
- all existing tests pass;
- all new runtime-parameter tests pass;
- `run_examples.cmd` passes;
- encoder/decoder symmetry tests pass;
- generated-code tests pass;
- documentation is synchronized with the implementation;
- no obsolete parameter implementation remains in source or documentation;
- no runtime layer depends on AST, semantic or IR implementation types.

## 23. Historical deferred items

The following remain outside the 0.9.29 runtime-parameter closure unless separately added to the implementation contract:

- parameter range annotations for finite static layout bounds;
- parameter default values;
- mutable parameters;
- parameter structs/arrays;
- generated `$size_in_bytes`, `$min_size_in_bytes`, `$max_size_in_bytes`;
- virtual fields, aliases and transforms;
- complete attribute semantics;
- byte-order default/inheritance completion;
- `requires` completion;
- remaining variant semantics;
- callback/backend completion not directly required by parameter transport.

## 24. Architectural invariant

Runtime parameters are an input to execution, not a new semantic layer.

The authoritative chain remains:

```text
source parameter declaration
    ↓
canonical SymbolId
    ↓
core::Expr reference
    ↓
IR
    ↓
Plan parameter metadata
    ↓
execution-local SymbolId → Value environment
    ↓
shared runtime evaluator
```

There must be exactly one executable interpretation of a parameter-dependent expression.
