# EmbX Source Generator — AST Coverage Matrix

Status: 0.9.57 audit

This matrix is the maintenance checklist for the canonical source generator.
The generator consumes AST only; it does not reconstruct source from IR or Plan.

| AST construct | Canonical source form | Coverage |
|---|---|---|
| Module documentation | `/// ...` | covered |
| Namespace | `namespace ...;` | covered |
| Attribute declaration | `attribute ...;` | covered |
| Module endian | `@ endian ...` | covered |
| Type alias | `type ... = ...;` | covered |
| Struct / requirements | `struct ... { requires ...; ... }` | covered |
| Field / dimensions | field declaration + modifiers | covered |
| Bit field / bits block | `bits { ... }` | covered |
| Virtual field | `let ... = ...;` | covered |
| Field alias | `alias ... = ...;` | covered |
| Transform | `transform name(...)` | covered |
| Variant / cases / default | `variant ... by ... { ... }` | covered |
| Conditional | `if (...) { ... } else { ... }` | covered |
| Block | `block ... [...] { ... }` | covered |
| At | `at(...) { ... }` | covered |
| Align | `align(...);` | covered |
| Callback declaration/use | `callback ...` | covered |
| Parameter | `param ...;` | covered |
| Constant / computed constant | `const` / `computed` | covered |
| Enum / enum items | `enum ... { ... }` | covered |
| Expressions | canonical precedence + parentheses | covered |
| Terminated sequence | `until ... max ...` | covered; incomplete AST rejected |
| Attributes / documentation | canonical rendering | covered |
| Unsupported member subtype | explicit deterministic diagnostic | covered |

## Deliberate non-reconstruction

`Module::attributes` is not a source construct in the current grammar and is
therefore rejected explicitly. Import declarations are not AST nodes after the
current canonical parser/import pipeline and are not reconstructed from Plan or
other downstream representations.

## Acceptance checks

- Current example/course corpus round-trips.
- Generation is deterministic and idempotent.
- Malformed/incomplete AST state fails without partial successful output.
- Unsupported AST member kinds fail explicitly.
- No semantic, name-resolution, layout, or Plan logic is owned by the generator.
