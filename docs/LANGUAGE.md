# EmbX Language Contract

This is the current language contract represented by `grammar/EmbX.g4` and the semantic implementation.

## 1. Module structure

A module may contain, in source order:

```text
module documentation?
namespace?
import*
item*
```

Items are:

- attribute declarations;
- endian directive;
- `const` / `computed` constants;
- enums;
- structs;
- callback declarations;
- type aliases.

### Namespace and imports

```embx
namespace app::packet;
import "common.embx";
import "other.embx" as alias;
```

Qualified names use `::`.


### Conditional fields

A struct may contain a conditional member:

```embx
struct Packet {
  flags: u8;
  if (flags == 1) {
    value: u32;
  } else {
    small: u16;
  }
}
```

The condition must be boolean and may use values already available before the conditional. Names declared inside a branch are branch-local and are not available after the conditional. An `if` without `else` contributes zero bytes when false; layout bounds are derived from both alternatives using the canonical `LayoutBounds` model.

## Virtual fields

A struct may contain a read-only virtual field using `let`:

```embx
struct Packet {
  value: u8;
  let doubled = value * 2;
}
```

A virtual field has no wire representation and therefore contributes zero bytes to layout. Its expression is evaluated in declaration order after the fields and virtual fields it references are available. It may be referenced by later layout expressions, conditions, offsets, lengths, and other expressions. Forward references to later members are rejected by the executable Plan dependency validation.

The reference encoder evaluates virtual fields only to populate its execution environment; callers do not provide virtual-field values. The reference decoder evaluates and exposes the virtual value in the decoded object. Virtual fields are currently read-only. Field aliases are supported as a separate member form. They remain zero-width and read-only in the decoded model, while the encoder additionally accepts the alias as a writable input name for an earlier physical field. Transforms are supported on physical fields through the 0.9.34 `scale(factor)` contract.

## 2. Types

Primitive scalars:

```text
u8 i8 u16 i16 u32 i32 u64 i64 f32 f64
```

Terminal built-ins:

```text
bytes
string
```

A terminated sequence is a general binary sequence whose boundary is defined by a compile-time terminal byte sequence and a mandatory maximum payload length:

```embx
name: bytes until 0x00 max 256;
line: bytes until 0x0D 0x0A max 1024;
items: Item[*] until 0xFF 0xFF max 4096;
```

The logical value excludes the terminator, while the wire extent includes it. For structured sequences the terminator is recognized only at an element boundary; it is not a raw search across nested element representations. Failed decode restores the sequence cursor. `bytes until ...` remains the compact byte form; no string-specific terminated type exists. See `TERMINATED_SEQUENCE_SPEC.md`.

User-defined aliases, structs and enums are named types.

## 3. Type and field lengths

A type suffix is `[expr]`:

```embx
values: u16[4];
```

A field length modifier may also supply the length:

```embx
values: u16 [count];
```

The semantic representation normalizes these into `core::Type::dimensions`.

`[*]` means remaining extent and is represented as `Dimension::Remaining`:

```embx
data: bytes[*];
```

The semantic/Plan implementation permits remaining byte/string sequences only in a bounded context such as a block or other bounded operation.

## 4. Expressions

Expressions support:

- integer, hexadecimal, floating and string literals where permitted;
- identifiers and qualified names;
- parentheses;
- unary minus;
- `* / %`;
- `+ -`;
- `== != < <= > >=`;
- `&& ||`.

Expression identifiers are resolved to SymbolIds before Plan execution.

## 5. Constants

```embx
const VERSION = 4;
computed TOTAL = VERSION + 1;
```

Constants used by execution are materialized into Plan. Dependency cycles are rejected.

## 6. Enums

```embx
enum Protocol: u8 {
  ICMP = 1,
  TCP = 6,
  UDP = 17
}
```

The underlying type, when present, must resolve to a scalar integer type. If omitted, the current Plan representation uses `u32` as the default underlying execution type.

Enum values must fit the underlying type and must be numerically unique.

## 7. Structs and byte order

```embx
struct Header big {
  version: u8;
  length: u16;
}
```

Supported byte-order keywords are `little`, `big` and `native`. The current source syntax is the module directive `@ endian <order>;` plus struct and field/member endian modifiers. PlanBuilder resolves the effective order exactly once using module default → struct override → field override and stores the resolved value in Plan. `native` is preserved as an explicit Plan value and means host byte order. `$default byte_order` is not part of the current grammar.

## 8. Bit containers

```embx
bits {
  version: u8(4);
  flags: u8(4);
}
```

A bit container contains integer scalar fields only, has 1–64 total bits, and is stored in the smallest whole-byte container required by the total width.

## 9. Variants

```embx
variant body by kind {
  1: u32;
  2: {
    value: u16;
  }
  default: bytes[4];
}
```

Cases may contain a type or inline members. A default case is optional. Runtime selects a case by evaluating the already-resolved discriminator and case expressions.

## 10. Blocks, absolute regions and alignment

```embx
block payload[length] {
  data: bytes[*];
}

at(64) {
  marker: u32;
}

align(4);
```

Block sizes, offsets and alignments are integer expressions. Statically decidable values are materialized; runtime-dependent values remain explicit.

## 11. Callbacks

Declarations:

```embx
callback on_decode_packet(ctx);
callback on_encode_packet(ctx);
```

Uses:

```embx
callback on_decode_packet(kind);
callback on_encode_packet(kind);
```

Direction is determined by the `on_decode` / `on_encode` prefix and must agree with the declaration.

## 12. Attributes and documentation

Attributes are module-level semantic definitions and may be declared with one of four supported types:

```embx
attribute packed;
attribute width: u8;
attribute gain: f32;
attribute title: string;

[packed, width=8, gain=1.5, title="payload"]
struct Header { value: u8; }
```

An attribute use contains only its name and source value. The declaration is the sole authority for the declared type. Marker attributes cannot have values; typed attributes require a value of the declared lexical category. Duplicate uses and unknown attributes are semantic errors. Attribute uses are legal only at grammar-defined attachment points. Declarations and uses are preserved in Plan and reflection; runtime does not perform a second attribute-resolution step. The C++ backend exposes the accepted attribute metadata through the Plan contract but does not assign implicit C++ semantics to arbitrary user attributes.

Documentation comments use `///` or `/** ... */`. Module, declaration and member documentation is retained through Plan and reflection.

## 12a. Multidimensional array physical order

The order of array dimensions is semantically significant. For dimensions `D0, D1, ..., Dn-1`, dimension 0 is the outermost dimension and `Dn-1` is the innermost; the last dimension varies fastest on the wire.

For example, a 2 × 3 array is serialized as:

```text
A[0][0], A[0][1], A[0][2], A[1][0], A[1][1], A[1][2]
```

This order is independent of little-/big-endian byte order inside each element and independent of alignment/padding. EmbX currently has no language-level column-major or arbitrary-stride facility. External formats must be mapped explicitly to this contract. A consuming program may receive the decoded multidimensional data as a target-specific non-owning array view over the canonical contiguous element sequence; this is an API representation choice, not a language construct.

The complete normative definitions are `MULTIDIMENSIONAL_ARRAY_LAYOUT.md` and `MULTIDIMENSIONAL_ARRAY_VIEW.md`.

## 13. Logical layout range

Logical sizes, counts, extents, offsets, alignments and derived layout sizes use checked unsigned 64-bit semantics.

This does not imply unlimited allocation. Runtime and generated code convert logical quantities to host sizes only after checking the host range and configured resource limits.

## 14. Semantic restrictions

The grammar alone does not define all validity. Semantic/Plan validation additionally enforces:

- unique declarations and appropriate scope rules;
- valid named-type identity;
- alias-cycle rejection;
- integer requirements for sizes, offsets and alignments;
- legal bit widths;
- variant consistency;
- callback declaration/use consistency;
- dependency legality;
- layout safety and overflow checks;
- bounded-context requirements for remaining byte/string sequences.

## 15. Feature completion rule

A language feature is not complete until its semantics exist consistently through:

```text
Grammar → AST → Semantic → IR → Plan → Runtime/Backend → Tests → Documentation
```



## 15. Transforms

A transform is a value conversion attached to a physical field. The accepted 0.9.34 implementation supports `scale(factor)`.

```embx
struct Packet {
  temperature: i16 transform scale(0.1);
}
```

The transform does not change wire layout, field size, offset, alignment or layout dependencies. Decoder execution is `logical = wire × factor`; encoder execution is the checked inverse `wire = logical ÷ factor`. The factor is a finite, non-zero compile-time numeric constant. Runtime parameters and arbitrary transform expressions are not allowed.

The reference Encoder/Decoder, Reflection and generated C++ backend implement the accepted transform contract. Write-through aliases reuse the physical target transform; aliases and virtual fields do not own independent transforms.

## 15. `requires`

A struct may contain one or more struct-level requirements:

```embx
struct Packet {
  value: u16;
  requires value == 7;
}
```

A requirement is a boolean semantic constraint on the completed struct execution environment. It consumes no wire space and does not change layout bounds, offsets, alignment, byte order or `$next` semantics. Requirements may reference declared value symbols that are available in the completed Plan environment, including fields, aliases, virtual fields, module constants, enum values and runtime parameters; source declaration order does not impose an additional requirement-order restriction. Unknown symbols are rejected during Plan construction.

The reference Encoder and Decoder evaluate requirements transactionally after the required values are available; a failed requirement is a constraint violation. Reflection exposes canonical requirement expression text. Generated C++ supports the defined generated-value subset and rejects unsupported requirement forms explicitly rather than changing their semantics.

## 16. Generated size properties

The language provides generated layout properties:

- `$size_in_bytes` — the exact size in bytes, available when the layout is statically exact;
- `$min_size_in_bytes` — the canonical minimum layout size;
- `$max_size_in_bytes` — the canonical finite maximum layout size.

The properties are derived exclusively from the Plan `LayoutBounds` model. They do not create fields, consume layout space, or introduce a second runtime size algorithm. `$max_size_in_bytes` is unavailable for unbounded layouts, while `$size_in_bytes` is unavailable when minimum and maximum differ.

The values use checked `uint64_t` layout semantics. Within execution of a struct, the properties are available to canonical expressions through their built-in semantic identities.

## 17. Current non-features

The following remain explicitly deferred:

- `$default byte_order` syntax;
- runtime-dependent transform factors;
- arbitrary user-defined transform expressions/functions.

These are planned or explicitly deferred work, not accepted current syntax.

## Runtime parameters (0.9.29)

Module-level runtime parameters use `param name: type;`. The initial implementation accepts scalar primitive types `u8/u16/u32/u64`, `i8/i16/i32/i64`, `f32/f64`. Parameters receive canonical `SymbolId`s and are available to executable expressions without becoming sequential fields. Encoder and Decoder receive execution-local parameter environments; missing, extra, mismatched and out-of-range values are rejected.


### Field aliases

A field alias gives a second name to an earlier field:

```embx
struct Packet {
  value: u8;
  alias value2 = value;
}
```

A field alias consumes no bytes, has its own `SymbolId`, is available to later expressions, and is materialized by the reference encoder/decoder and reflection. During encoding, the alias may also be supplied as the input name for an earlier physical field; the target field may then be omitted. If both names are supplied, their scalar values must agree. A write-through alias target must be a physical field; a virtual field cannot be a writable alias target. If the physical target has a transform, the alias reuses that target transform and does not introduce a second transform.
