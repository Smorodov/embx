# EmbX course — introduction and common workflow

This document is the common introduction for the whole course. It explains what the course is teaching, how an EmbX program moves through the compiler, what artifacts can be obtained from it, and how to work through the lessons without repeating the same background in every lesson.

The individual lessons should concentrate on one language concept. General workflow, terminology, validation rules and common commands belong here.

## 1. What the course teaches

EmbX is a declarative language for describing binary data formats. A lesson therefore does not start by writing a conventional parser. It starts by describing the structure of bytes and the meaning of the fields in an `.embx` source file.

The course follows one common path:

```text
EmbX source
    ↓
Parser / AST
    ↓
Semantic analysis
    ↓
IR
    ↓
Plan
    ↓
Reference execution / generated backend / reflection
    ↓
Binary bytes and observable results
    ↓
Test or external validation
```

The exact final step depends on the lesson. A small artificial format can be checked by an encode/decode round-trip. A real format can additionally be checked against an existing binary file or an external tool.

The important point is that the lesson is not a second implementation of the language. The same compiler and semantic pipeline used by the project tests are used by the course.

## 2. The basic idea: describe the format, do not hand-write the parser

Suppose a binary message contains:

1. two fixed bytes identifying the format;
2. one integer containing a version;
3. four bytes of payload.

The EmbX description expresses those facts directly. The compiler resolves the description into its canonical semantic representation and Plan. The reference runtime and backends then execute that resolved meaning.

A lesson should therefore answer four questions:

- What bytes exist?
- What does each part mean?
- How is the meaning represented in EmbX?
- How can we prove that the resulting behavior is correct?

## 3. The compiler pipeline

### 3.1 Source

The input is an `.embx` file containing declarations of types, structures, fields, expressions and other language constructs supported by the current language contract.

Example:

```embx
struct Message {
    magic: bytes[2] = "MX";
    version: u8;
    payload: bytes[4];
}
```

The source is the only format description written by the user. The rest of the pipeline derives its meaning from this description.

### 3.2 Parser and AST

ANTLR parses the source according to `grammar/EmbX.g4`. The result is an AST containing the syntactic structure of the source.

The AST is a compiler representation. It is not a runtime format description and is not carried into runtime execution.

For inspection, the command-line tool can print the AST:

```text
embx examples/common.embx --dump-ast
```

This is useful when learning syntax or diagnosing a parser-level problem.

### 3.3 Semantic analysis

The semantic layer resolves the meaning of the parsed program. Among other things, it resolves names, types, expressions, dependencies, layout information and canonical symbol identity.

This is where a source construct becomes a well-defined language construct rather than merely valid syntax.

A lesson normally does not need to explain the internal implementation of semantic analysis. It needs to explain the language rule being learned and show its observable consequence.

### 3.4 IR

The compiler IR is a self-contained resolved compiler representation. It contains canonical types, expressions, resolved `SymbolId` references, declaration information and nested operations.

IR is still compiler data. It is not the runtime API and it is not a second public format description.

### 3.5 Plan

`Plan` is the executable semantic boundary of EmbX.

The Plan contains the resolved operations required to execute the format. The reference encoder, decoder, reflection layer and generated C++ backend consume the Plan instead of reconstructing language semantics independently.

This is one of the central architectural principles of the project:

> Resolve the meaning once; execute the resolved meaning afterwards.

### 3.6 Reference execution

The reference runtime provides the canonical execution path used by the course and conformance tests. It includes reference encoding and decoding and the runtime facilities required by the accepted Plan operations.

For a lesson, this path is especially important because it gives a direct way to test the language itself without first depending on a generated target backend.

### 3.7 Generated backend

The accepted C++ backend consumes the same Plan and produces target-side C++ representation and codec code.

The backend is not allowed to redefine EmbX semantics. It implements the already-resolved Plan semantics for its target environment.

The command-line tool can generate C++ from an `.embx` file:

```text
embx examples/common.embx --generate-cpp generated/common
```

This produces:

```text
generated/common.hpp
generated/common.cpp
```

The output prefix may include a directory. The directory must already exist.

### 3.8 Reflection

Reflection exposes selected resolved Plan facts such as members, symbols and layout information. It is useful when a lesson needs to demonstrate what the compiler knows about a format without executing the complete codec.

Reflection is an inspection surface, not a second semantic model.

## 4. What we can get from one EmbX description

Depending on the language constructs used and the current project stage, one description can provide several different results from the same resolved meaning:

| Result | Purpose |
|---|---|
| Parse result / AST | inspect syntax and source structure |
| Semantic result | verify that names, types, expressions and dependencies are valid |
| Plan | executable canonical meaning |
| Reference encode | turn a logical value into binary bytes |
| Reference decode | interpret binary bytes according to the Plan |
| Reflection | inspect resolved members, symbols and layout facts |
| Generated C++ | obtain target-side representation and codecs |
| Tests | automatically prove a defined contract |
| Real binary fixture | compare the description with concrete bytes |
| External result | validate behavior outside the compiler when the format permits it |

Not every lesson needs every result. A lesson should use the smallest set that proves the concept being taught.

## 5. What the command-line program currently does

The `embx` command-line executable is intentionally small. Its current public operations are:

```text
embx <file.embx>
embx <file.embx> --dump-ast
embx <file.embx> --generate-cpp <output-prefix>
embx --help
embx --version
```

With only the source file, the command compiles it and reports successful parsing/compilation.

`--dump-ast` prints the parsed AST.

`--generate-cpp` compiles the source through the compiler pipeline and generates the C++ header/source pair from the resulting Plan.

Reference encode/decode operations used by the tests and lessons are currently library/test APIs rather than separate command-line subcommands. The course should not pretend that a CLI command exists when the current executable does not provide it.

## 6. The common working cycle

Every practical lesson should use approximately the following cycle.

### Step 1 — understand the bytes

Before writing EmbX, define the format in concrete terms.

For example:

```text
byte 0..1   magic
byte 2      version
byte 3..6   payload
```

For dynamic or conditional formats, also state the rule that determines size, presence or interpretation.

### Step 2 — write the smallest EmbX description

Use only the language constructs required for the lesson. Do not introduce a more complicated construct merely because a later lesson will teach it.

### Step 3 — compile it

Use the project build and/or the CLI to verify that the source is accepted.

For direct inspection:

```text
embx path/to/example.embx
embx path/to/example.embx --dump-ast
```

### Step 4 — inspect the resolved behavior

When the lesson requires it, use the existing Plan, reference runtime, reflection or generated C++ test infrastructure to observe the resolved result.

### Step 5 — test binary behavior

Use concrete bytes. Prefer deterministic fixtures over vague demonstrations.

For a new artificial format, the normal first proof is:

```text
logical value
    ↓ encode
binary bytes
    ↓ decode
logical value
```

The decoded result must satisfy the lesson's contract.

### Step 6 — test the generated backend when appropriate

If the lesson is intended to cover generated C++, use the same source and verify that the generated codec agrees with the Plan/reference behavior.

### Step 7 — run the regression suite

The complete project acceptance command is:

```text
run_examples.cmd
ctest --test-dir build --output-on-failure
```

The exact build command is defined by the current project build scripts. The course does not replace the normal project acceptance process.

## 7. Encoding and decoding: the mental model

The course uses a simple distinction.

**Encoding** starts with a logical value and produces bytes according to the Plan.

```text
logical value → Plan execution → bytes
```

**Decoding** starts with bytes and produces a logical value according to the same Plan.

```text
bytes → Plan execution → logical value
```

The encoder and decoder are opposite execution directions of the same semantic model. They are not two independently designed format definitions.

For a deterministic fixed format, a successful round-trip should look like:

```text
value A
  → encode → bytes B
  → decode → value A'
```

where `A'` satisfies the lesson's equality/normalization contract for `A`.

## 8. Binary layout is part of the meaning

A field is not only a type name. Its position, width, byte order, alignment, conditions, dimensions and dependencies can affect the resulting binary layout.

When a lesson teaches layout, always make the byte-level consequence explicit.

For example:

```text
magic   : bytes[2]  → 2 bytes
version : u8         → 1 byte
payload : bytes[4]  → 4 bytes
                         -----
                         7 bytes
```

This makes a layout test understandable without reading compiler implementation code.

For dynamic layouts, do not assume a single static size. Distinguish exact, bounded and unbounded layout according to the language contract.

## 9. Compile-time and runtime information

One of the recurring ideas in the course is the difference between information known while compiling the format and information supplied while executing it.

Examples of compile-time information include:

- declared field and type names;
- constants and enum values;
- statically known dimensions;
- statically known offsets and alignments;
- resolved symbol identities.

Runtime information may include:

- actual field values;
- bytes being encoded or decoded;
- explicitly supplied runtime parameters;
- values used by dynamic expressions.

A lesson should always state which category a value belongs to when that distinction matters.

## 10. Errors and negative examples

A good course does not demonstrate only successful programs. Many language rules are best understood by showing what must be rejected.

There are several useful levels of failure:

1. **Syntax error** — the source does not match the grammar.
2. **Semantic error** — the syntax is valid but the program violates a language rule.
3. **Plan/compiler rejection** — a construct cannot be lowered into the accepted executable semantics.
4. **Runtime error** — execution cannot proceed for the supplied data or execution inputs.
5. **Conformance failure** — execution succeeds but the produced or consumed bytes do not satisfy the external format contract.

Tests should identify the level being demonstrated. Do not turn a runtime failure into an apparently valid language rule, and do not hide a semantic rejection behind a generic error.

## 11. How to read a lesson

Every active lesson should answer the same questions in roughly the same order:

1. **Goal** — what concept are we learning?
2. **Binary format** — what concrete bytes are involved?
3. **EmbX source** — what is the smallest description?
4. **Pipeline** — what happens to that source?
5. **Execution** — what value or bytes do we obtain?
6. **Proof** — which test or external observation proves it?
7. **Failure cases** — what should be rejected or fail?
8. **Takeaway** — what should be remembered before moving on?

Common compiler architecture details should not be copied into every lesson. This introduction is the shared reference for them.

## 12. How the lessons build on each other

The order is deliberate. Later lessons should reuse concepts already established rather than introducing an independent mechanism.

```text
01  First binary format
 ↓
02  Integers and byte order
 ↓
03  Bit fields
 ↓
04  Arrays and dynamic dimensions
 ↓
05  Nested structures
 ↓
06  Variants and conditionals
 ↓
07  Offsets and alignment
 ↓
08  Symbol dependencies and $next
 ↓
09  Virtual fields and aliases
 ↓
10  Callbacks and transforms — accepted
 ↓
11  IPv4
 ↓
12  General terminated sequences
 ↓
13  MIDI
 ↓
14  Container / packet format
 ↓
15  Callback protocol
 ↓
16  Complete protocol
```

The first lessons isolate language mechanics. Later lessons combine them. Real-format lessons demonstrate that the same mechanisms describe actual binary data rather than only artificial teaching examples.

## 13. What belongs in a lesson and what belongs here

### Put in this introduction

- common compiler pipeline;
- common terminology;
- general build and test procedure;
- common CLI commands;
- general encode/decode model;
- general error categories;
- common acceptance rules;
- how to read and validate a lesson.

### Put in an individual lesson

- the specific binary format;
- the exact EmbX constructs introduced there;
- the concrete source file;
- the concrete bytes or fixture;
- the lesson-specific Plan behavior;
- the lesson-specific positive and negative tests;
- any external validator or observable result.

### Put in normative language documentation

Language rules belong in `docs/LANGUAGE.md` and the relevant contracts. A lesson may explain and demonstrate those rules, but it must not silently redefine them.

## 14. Course acceptance chain

The standard evidence chain is:

```text
lesson
  ↓
EmbX source
  ↓
Plan
  ↓
reference execution
  ↓
generated backend (when applicable)
  ↓
binary fixture
  ↓
test / external validation
```

A lesson is complete when its claims can be reproduced through the project rather than merely read in prose.

The course therefore serves two purposes at once:

1. it teaches a reader how to use EmbX;
2. it continuously exercises the language, Plan and execution pipeline with small understandable examples.

## 15. Current starting point

At EmbX 0.9.52, Lessons 1–14 form the current accepted teaching sequence in this course: first binary format, integers and byte order, bit fields, arrays and dynamic dimensions, nested structures, variants and conditionals, offsets and alignment, symbol dependencies and runtime parameters, virtual fields and aliases, callbacks and transforms, IPv4, and general terminated sequences, MIDI, and TLV composition. Lesson 13 is MIDI, the first real-format conformance lesson; Lesson 14 composes existing length-dependent and terminated-sequence mechanisms into a small TLV format.

Start with Lesson 1 after reading this introduction, then continue through the active sequence in order. The introduction is deliberately not a replacement for the lessons: it supplies the common map so each lesson can concentrate on its own language concept.


## Lesson 11 — IPv4

Accepted in 0.9.47 and retained through 0.9.52. The lesson expresses a fixed 20-byte IPv4 header using existing EmbX primitives and validates it against a real binary fixture. It intentionally does not introduce IPv4-specific semantics.

## 16. Current practical sequence

The active practical sequence now places generalized terminated sequences before the real-format MIDI lesson:

```text
Lesson 11 — IPv4
Lesson 12 — General terminated sequences
Lesson 13 — MIDI
Lesson 14 — TLV composition
```

Lesson 12 is deliberately format-neutral. It teaches a general binary rule already present in the
language contract: a terminated sequence may contain structured elements, and a containing terminator
is recognized only between complete elements. MIDI remains the first lesson whose main goal is
external real-format conformance and playback.

## Lesson 14 — TLV composition

Lesson 14 builds a small Type–Length–Value format from existing EmbX mechanisms: integer fields,
expression-dependent byte arrays, a remaining structured sequence, and a terminated outer sequence.
It intentionally introduces no TLV-specific compiler semantics. The lesson uses a real binary fixture,
checks an inner `FF FF` payload against the outer sequence terminator rule, and exercises transactional
failure when a declared payload length exceeds the available input.
