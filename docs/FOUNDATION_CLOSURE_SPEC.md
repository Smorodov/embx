# EmbX Foundation Closure Specification

**Current foundation baseline:** 0.9.49 — AST → Plan → reference-runtime architecture remains closed; the executable course through Lesson 11 and terminated-sequence contract are locally validated at 73/73 CTest tests

## 1. Objective

This document is the normative foundation contract; current language status is tracked by `LANGUAGE.md` and `LANGUAGE_COMPLETION_MATRIX.md`. Historical milestone audits are intentionally not part of the release source tree.

Close the compiler foundation before adding further language features. The foundation must have one authoritative representation for every executable semantic fact and one deterministic pipeline from source to executable Plan.

## 2. Pipeline invariant

```text
Grammar → AST → Semantic → IR → Plan → Runtime / Encoder / Decoder / Reflection / Backend
```

Downstream layers must not reconstruct semantic facts from source spelling, parser state, or obsolete intermediate representations.

## 3. Expression closure

`core::Expr` is the canonical executable expression representation after semantic binding. Identifier expressions carry `SymbolId`; source text may remain for diagnostics.

The semantic layer defines canonical expression result types and explicit rules for unary operators, arithmetic, comparisons and logical operators. Generic/core inference may use `Unknown` before declaration information is available, but after SymbolId binding a declaration with a known semantic type must be resolved before Plan-time operator validation. The result types are signed integer, unsigned integer, floating, boolean, string, unknown and invalid. `core::Expr` is the only executable expression tree; the former AST-local expression struct is removed.

`&&` and `||` require boolean operands and are short-circuiting. `%` requires integer operands. Comparisons are numeric. Unknown identifier types may remain deferred only while their declaration type is genuinely unavailable at the current semantic boundary. Once a valid SymbolId identifies a declaration with a known type, that type must participate in operator validation. Statically incompatible operands must be rejected before executable Plan acceptance.

Integer operations use checked semantics and must never silently wrap or convert a negative value into an unsigned layout quantity. Constant evaluation and runtime evaluation use the same runtime evaluator and therefore share the same arithmetic and short-circuit behavior.

Layout expressions for offsets, sizes, alignment, counts and extents must produce an integer result suitable for checked `uint64_t` logical quantities.

## 4. Type-shape closure

Executable type shape is represented canonically. Fixed, dynamic and remaining dimensions are not duplicated across independent AST properties. Source syntax may remain distinct, but semantic payload must have one authoritative normalized representation.

## 5. Field value constraints

A field literal assignment such as:

```embx
magic: u16 = 0xA1B2;
```

is a field value constraint/assertion. The implementation must not continue to describe the same executable semantic fact as a Plan assertion while exposing an unrelated historical `constant` concept without an explicit contract.

Encoder and decoder must enforce the same constraint semantics.

## 6. Layout closure

Layout operations must have a unified model for logical offset, logical size, dependencies and static/dynamic classification.

All operations involving layout arithmetic use checked unsigned 64-bit semantics:

- `offset + size`;
- `size * count`;
- alignment padding;
- nested extents;
- derived sizes.

Host `size_t` conversions are checked separately.

### 6.1 Canonical layout bounds

Plan exposes `LayoutBounds { minSize, maxSize }` as the single static size classification. A missing `maxSize` means `Unbounded`; a finite equal pair is `Exact`; a finite unequal pair is `Bounded`. Fixed dimensions, scalar types, bit storage and statically sized blocks produce exact bounds. Runtime-dependent dimensions, remaining dimensions and runtime-sized blocks produce an unknown finite maximum unless a later semantic rule supplies one. Sequential layout accumulates bounds with checked addition; alignment is applied to the current cursor; `at(offset)` evaluates its nested sequence from the absolute offset, contributes that sequence's extent without advancing the outer sequential cursor, and remains absolute/root-relative when nested; variants take the minimum lower bound and maximum upper bound across reachable alternatives.

This bounds model is descriptive Plan metadata and must not become a second executable layout algorithm. Runtime execution continues to use the executable Plan operations and checked runtime expressions.

## 7. Dependency graph

`LayoutGraph` is the authoritative layout dependency graph. It must detect cycles, invalid dependencies and illegal dynamic/static combinations. A second independent dependency algorithm is forbidden.

## 8. `$next`

`$next` is implemented in 0.9.28. It is a built-in unsigned layout value valid only in `at(...)` offset expressions. It denotes the end of the immediately preceding sequential layout operation. Its canonical identity is carried through IR/Plan; runtime supplies the current sequential cursor, and static Plan layout materializes it when the cursor is known. It is never resolved through runtime string lookup.

## 9. Conditional fields

Conditional fields were introduced in 0.9.28 and are closed through Grammar, AST, Semantic, IR, Plan, static/dynamic layout rules, runtime Encoder/Decoder execution, reflection and regression tests. Backend support remains subject to the documented backend coverage matrix.

## 10. Runtime parameters

Runtime parameters are typed, SymbolId-bound execution inputs. They are part of the runtime environment and may participate in canonical expressions. They must not expose AST or semantic implementation details to runtime.

## 11. Size properties

`$size_in_bytes`, `$min_size_in_bytes` and `$max_size_in_bytes` are generated properties derived exclusively from Plan/layout semantics. Independent ad-hoc runtime size algorithms are forbidden. `$size_in_bytes` requires an exact bound; `$min_size_in_bytes` is always the canonical lower bound; `$max_size_in_bytes` requires a finite bound.

## 12. Byte order

Byte-order inheritance is resolved once into the Plan. The precedence rule is module default → struct override → field override. `native` is an explicit resolved value meaning host byte order. Runtime and generated backends must consume the resolved Plan value and must not re-run inheritance.

## 13. Attributes and requires

Attributes require a canonical declaration/use model, validation, legal attachment rules and reflection behavior. `requires` is now the accepted 0.9.38 feature: a struct-level semantic constraint backed by canonical expressions, represented in Plan, exposed through reflection, and enforced symmetrically by the reference runtime and generated backend where supported. See `docs/REQUIRES_CONTRACT.md`.

## 14. Variants and callbacks

Variants require complete discriminator, case, default, dependency, size and backend semantics. Callbacks require canonical identity; string names are permitted only at explicit external registration/API boundaries.

## 15. Cleanup rule

Mechanical cleanup must remove dead helpers, empty translation units, stale comments, obsolete compatibility paths, unused build variables and contradictory active documentation. Historical snapshots belong outside the active source tree.

## 16. Completion gate

Foundation Closure is complete only when:

- canonical expressions are established;
- expression typing and checked arithmetic are complete;
- type shapes and field constraints are canonical;
- static/dynamic layout is formally defined;
- layout dependencies are authoritative;
- `$next` is implemented;
- conditional fields and runtime parameters are complete across applicable execution layers;
- generated size properties are complete;
- byte-order, attributes and `requires` semantics are complete;
- variants and callbacks are complete across applicable execution layers;
- tests cover positive, negative, boundary, overflow, determinism and symmetry behavior;
- active documentation and the completion matrix agree with the source tree;
- a clean reference build passes all tests and canonical examples.


Conditional members use `if (expr) { ... }` with an optional `else { ... }`. The condition is a canonical executable `core::Expr` and must have boolean type. Each branch is lowered into an independent member scope; branch-local fields are not available in the enclosing scope after the conditional. Conditions may reference only values already available at the point of the conditional.

The executable Plan represents a conditional as one `Conditional` operation containing the condition and the two branch operation sequences. Layout bounds reuse the canonical `LayoutBounds` model: a missing `else` contributes a zero-sized false branch; with both branches present, minimum and maximum are composed across both alternatives. Runtime Encoder/Decoder execution is implemented in the accepted conditional-runtime baseline and covered by regression tests; backend coverage remains subject to the completion matrix.


## 0.9.49 accepted green

Lesson 11 uses the existing Plan semantics to describe a fixed 20-byte IPv4 header. Terminated sequences add a bounded sequence-boundary rule for byte and structured elements while preserving the foundation invariants: one canonical type representation, one Plan layout model, one reference execution path, and no parallel resolver/evaluator/cursor mechanism.
