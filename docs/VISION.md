# EmbX Vision

This document describes the project vision and design principles. It is intentionally non-normative. The normative language meaning remains defined by `docs/LANGUAGE.md` and the associated semantic/Plan contracts.

## 1. What EmbX is

EmbX is a declarative language for describing binary data formats and the operations needed to work with them.

A format description should be understandable as a description of the data itself, not as an implementation of a parser or serializer. From one EmbX description the compiler derives a canonical semantic model, an executable Plan, and the information required by reference execution and target backends.

The central idea is:

```text
binary format description
        ↓
     EmbX source
        ↓
 canonical meaning
        ↓
   executable Plan
        ↓
 reference execution / generated backend
```

EmbX therefore treats the format description as the source of truth. Encoding, decoding, reflection and generated code are different ways of executing or exposing the same resolved meaning.

## 2. The problem EmbX addresses

Binary formats are often implemented as hand-written code. That approach mixes several different concerns:

- what the bytes mean;
- how fields are laid out;
- how names and dependencies are resolved;
- how runtime conditions are evaluated;
- how bytes are read and written;
- how the result is exposed to an application;
- how the implementation is generated for another language or platform.

When these concerns are duplicated, different implementations can silently acquire different semantics.

EmbX is intended to separate these concerns while keeping one authoritative meaning for the format.

## 3. The central architectural principle

**Describe once, resolve once, execute the resolved meaning.**

There must be one authoritative semantic interpretation of an EmbX program.

The compiler resolves source names, types, expressions, dependencies and layout into a canonical Plan. Everything after that boundary consumes the Plan rather than reconstructing language semantics.

This gives the project a simple dependency direction:

```text
syntax
  ↓
semantic meaning
  ↓
canonical IR
  ↓
executable Plan
  ↓
execution / reflection / code generation
```

No later layer should become a second compiler.

## 4. General principles

### 4.1 One source of truth

A semantic fact should have one authoritative representation.

If a fact already exists in the Plan, another layer must not independently derive a competing version of that fact.

### 4.2 One identity system

`SymbolId` is the canonical internal identity of declarations.

Names are source-level and diagnostic information. Once a declaration has been resolved, execution uses its identity rather than searching by spelling again.

### 4.3 Plan is the execution boundary

The Plan is the boundary between understanding a language and executing a format.

Before Plan creation, the compiler may answer semantic questions. After Plan creation, runtime components execute already-resolved operations.

### 4.4 Minimality and sufficiency

EmbX should contain the smallest set of concepts that is sufficient to describe and execute useful binary formats correctly.

A new abstraction is justified only when an existing abstraction cannot express the required semantics without ambiguity or duplication.

In particular, the project avoids:

- duplicate semantic models;
- second expression representations without a clear ownership boundary;
- backend-specific semantic resolvers;
- duplicate layout algorithms;
- compatibility layers for obsolete models;
- format-specific semantics hidden inside the compiler core.

### 4.5 Declarative first

The EmbX source should say **what the format is**, not reproduce the control flow of a hand-written parser.

Implementation mechanisms belong to the compiler, Plan, runtime or backend rather than to every format description.

### 4.6 Deterministic semantics

The same source, compiler rules and inputs should produce the same semantic result.

Ordering that has semantic meaning is explicit and deterministic. Unordered containers must never accidentally define language behavior.

### 4.7 Explicit boundaries

Each layer owns a limited responsibility:

- grammar/parser — syntax;
- AST — source structure;
- semantic analysis — meaning and legality;
- IR — resolved compiler representation;
- Plan — executable semantics;
- runtime — execution against data and runtime state;
- backend — target-specific implementation.

Crossing these boundaries for convenience is discouraged because it tends to create duplicated semantics.

### 4.8 Binary correctness first

A format description is useful only if it corresponds to real bytes.

EmbX development therefore validates the language against concrete binary fixtures, encode/decode symmetry, negative cases and, where possible, an independent external or observable result.

The MIDI example is the first explicit demonstration of this principle: the description is connected to real Standard MIDI File bytes and an audible result.

### 4.9 Safe arithmetic and explicit limits

Logical binary-layout quantities use checked 64-bit arithmetic. Large logical ranges must not silently become unsafe host-memory operations.

The distinction between logical format capacity and physically available memory remains explicit at runtime.

### 4.10 Backends implement semantics; they do not redefine them

A C++ generator, future Rust generator, Python generator, reflection layer or another backend may use different implementation techniques.

They must nevertheless implement the same Plan semantics.

Backend diversity is therefore an implementation concern, not a reason to duplicate the language definition.

## 5. What EmbX is not

EmbX is not intended to be:

- a general-purpose programming language;
- a replacement for application business logic;
- a collection of independent parser generators with subtly different semantics;
- a format-specific DSL disguised as a general language;
- a framework in which every backend is allowed to interpret the source independently.

Application-specific processing can surround EmbX execution. The EmbX core remains responsible for describing and executing the binary representation itself.

## 6. What a good EmbX description should feel like

A good format description should make the binary structure visible directly in the source.

For example:

```embx
struct Message {
  magic: bytes[4] = "MSG0";
  version: u8;
  length: u16;
  payload: bytes[length];
}
```

The important information is visible:

- the byte signature;
- the field order;
- the field widths;
- the dependency between `length` and `payload`.

The compiler is responsible for turning those declarations into validated executable operations.

## 7. Long-term direction

The long-term goal is a language whose core semantics are stable enough that one format description can serve several purposes at once:

1. define the binary representation;
2. validate the format statically where possible;
3. execute decoding and encoding through the reference implementation;
4. generate efficient target-specific implementations;
5. expose structural information through reflection;
6. provide executable documentation and conformance tests;
7. make real binary fixtures part of the specification rather than after-the-fact examples.

The reference implementation remains important even when generated backends become mature. It provides an unambiguous semantic oracle for conformance and debugging.

## 8. How the project should evolve

Language growth should follow the same discipline as the current foundation:

```text
real requirement
      ↓
minimal language concept
      ↓
semantic contract
      ↓
Plan representation
      ↓
reference execution
      ↓
backend implementation
      ↓
executable test / real bytes
```

A feature is not complete merely because syntax exists or one backend can implement it.

A feature becomes part of the language when its meaning is defined, represented canonically, executable through Plan, tested, documented and consistent across applicable backends.

## 9. The course as part of the vision

The `course/` tree is not a separate educational implementation. It is executable documentation for the language.

Each lesson should answer the same question from a progressively larger perspective:

> How does a small declarative description become correct, executable binary behavior?

The intended evidence chain is:

```text
lesson
  ↓
EmbX source
  ↓
Plan
  ↓
reference execution
  ↓
generated backend
  ↓
real bytes / fixture
  ↓
test or external observation
```

This keeps learning material, examples and implementation semantics aligned.

## 10. Definition of success

EmbX succeeds when a developer can read a format description and trust that:

- its structure is explicit;
- its semantic dependencies are well-defined;
- its layout is checked;
- its reference execution is deterministic;
- generated implementations have the same meaning;
- real binary fixtures can validate the description;
- documentation and executable behavior remain synchronized.

The goal is not to maximize language size. The goal is to make binary-format descriptions **clear, precise, executable and portable without duplicating their meaning**.
