# EmbX Expression Semantics

**Contract baseline:** 0.9.46; retained unchanged in 0.9.48.

**Status:** accepted; expression typing is closed through executable Plan validation.

## Canonical representation

`core::Expr` is the sole executable expression representation. `ast::Expr` is an alias of `core::Expr`; there is no second AST expression structure. After IR lowering, identifiers carry their canonical `SymbolId`. Source text is retained only for diagnostics/source presentation.

## Result types

Every expression has one of:

- `IntegerSigned`
- `IntegerUnsigned`
- `Floating`
- `Boolean`
- `String`
- `Unknown` — an identifier whose declared/value type is not yet available at the current layer
- `Invalid`

Literal rules:

- decimal integer within `int64_t` range → signed integer;
- decimal integer above `int64_t` range → unsigned integer;
- hexadecimal integer → unsigned integer;
- decimal floating literal → floating;
- quoted literal → string;
- `true` / `false` → boolean.

## Operators

Unary `-` accepts integer and floating operands. Unsigned integer negation produces a signed integer and is checked during evaluation.

Arithmetic `+ - * / %` requires numeric operands. The result is floating if either operand is floating; otherwise it is unsigned if either operand is unsigned; otherwise signed. `%` is integer-only.

Comparison `== != < <= > >=` requires numeric operands and produces boolean.

Logical `&& ||` requires boolean operands and produces boolean. Both operators are strictly short-circuiting: the right operand is not evaluated when its value cannot affect the result.

## Checked arithmetic

Signed overflow, unsigned overflow and unsigned underflow are errors. Division or remainder by zero is an error. `INT64_MIN / -1` is an overflow.

Mixed signed/unsigned operations are deterministic: a negative signed operand cannot participate in a mixed integer operation; non-negative signed values are promoted exactly to unsigned for the operation.

## Layout expressions

Layout quantities such as offsets, sizes, counts, extents and alignment are evaluated as integers and converted to checked logical `uint64_t`. Negative values are rejected; no implicit signed-to-unsigned wraparound is permitted.

## Resolved identifier typing

Generic `core::Expr` inference may use `Unknown` when declaration information is not available at that layer. After SymbolId binding, however, a referenced declaration with a known semantic type must be resolved before executable Plan operator validation. `Unknown` must not silently bypass statically determinable operand checks. The existing variant-discriminator type-resolution path is the reference direction for this closure.

## Evaluation boundary

Plan construction and runtime use the same expression evaluator. The Plan may materialize values that are statically decidable; dynamic expressions remain canonical `core::Expr` trees with bound `SymbolId` references.
