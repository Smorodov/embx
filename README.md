# EmbX

**EmbX is a language and toolchain for describing binary data formats and turning those descriptions into executable encoding/decoding plans.**

EmbX is designed for binary protocols and structured binary files where the important thing is not only the wire layout, but a precise, testable semantic model of that layout.

> **Current public-preparation release: EmbX 0.9.86**  
> **Based on accepted EmbX 0.9.85 GREEN — 91/91 CTest tests passing (100%)**

## Why EmbX?

Binary formats are often implemented as hand-written parsing code. That works, but the description of the format, its layout rules, and its implementation can easily drift apart.

EmbX takes a different approach:

```text
EmbX source
    ↓
   AST
    ↓
Semantic analysis
    ↓
    IR
    ↓
   Plan
    ↓
Reference Runtime / generated C++ / reflection
```

The **Plan** is the executable semantic boundary. The reference runtime provides the canonical execution path, while generated backends implement the same Plan semantics independently.

## A small example

An EmbX description can stay close to the actual binary format:

```embx
struct Packet big {
    magic: bytes[4] = "PKT0";
    version: u8;
    length: u16;
    payload: bytes[length];
}
```

The source describes the structure and its wire representation instead of embedding the format rules in a hand-written decoder.

EmbX also supports existing constructs such as:

- fixed and dynamic arrays;
- bounded byte sequences;
- nested structures;
- variants;
- bit fields;
- explicit byte order;
- alignment and `at(...)` layout;
- `$next` sequential layout references;
- imports and qualified names;
- aliases and virtual fields;
- scalar transforms such as `scale(...)`;
- runtime parameters;
- encode/decode callbacks;
- documentation and attributes.

See [`examples/`](examples/) for the executable example corpus.

## Quick start

The current development environment is Windows + MSYS2/UCRT64. The repository includes ready-to-use command files for the standard workflow.

### Prerequisites

- Windows
- MSYS2 UCRT64
- GNU C++ compiler
- CMake
- Java 11 or newer
- ANTLR 4.13.x
- Catch2 3.x

The exact environment used for the current verified baseline includes GNU 16.2.0, Java 17 and ANTLR 4.13.2.

### Build and test

From the repository root:

```cmd
build.cmd
```

For the complete test gate:

```cmd
build_all.cmd
```

The accepted 0.9.85 baseline passes:

```text
91/91 tests passed
100% tests passed
```

### Run the examples

```cmd
run_examples.cmd
```

The examples are not just documentation snippets: they are parsed and semantically checked by the real EmbX CLI.

There is also a practical MIDI example:

```cmd
examples\play_midi.cmd
```

It uses `examples/midi_demo.mid` for an audible Windows-side format check.

## Repository layout

```text
EmbX/
├── grammar/       EmbX grammar
├── src/           compiler, semantic model, Plan, runtime and generators
├── examples/      small executable language examples
├── tests/         unit, contract and differential tests
├── course/        educational material and external-format adapters
├── docs/          architecture, contracts, roadmap and development notes
└── CMakeLists.txt build and test configuration
```

## Testing philosophy

EmbX uses the **Reference Runtime as the canonical execution path** for conformance. Generated C++ is tested against it rather than being allowed to define its own semantics.

This makes a differential test answer a precise question:

> Does generated C++ implement the same semantics as the canonical Plan execution path?

The test suite covers parsing, semantic analysis, layout, runtime behavior, generated-code contracts, hardening and generated-C++ differential conformance.

## Architecture principles

The project deliberately keeps the semantic core small. The main invariants are:

1. **Plan is the executable semantic boundary.**
2. **SymbolId is the canonical internal identity.**
3. **There is one name-resolution model.**
4. **There is one canonical type-shape model.**
5. **There is no second layout engine in a backend.**
6. **Backends do not repair or redefine language semantics.**
7. **Checked 64-bit logical layout arithmetic is used for layout calculations.**
8. **External formats stay outside the semantic core.**
9. **New language features require a real semantic gap, not a feature-count goal.**

These rules are important because EmbX is intended to remain understandable as the language and its generated backends grow.

## Generated languages

Generated C++ is the currently qualified backend and is part of the mandatory project roadmap.

Additional generated-language backends such as Rust or Python are intentionally **community-driven extensions**, not release blockers. The project does not add language constructs merely to make a particular additional backend possible.

## Documentation

The README is the entry point. Detailed technical information lives in [`docs/`](docs/). In particular:

- [`docs/DEVELOPMENT_ROADMAP.md`](docs/DEVELOPMENT_ROADMAP.md) — current development direction
- [`docs/DEVELOPMENT_HANDOFF.md`](docs/DEVELOPMENT_HANDOFF.md) — architecture and continuation rules
- [`docs/LANGUAGE_COMPLETION_MATRIX.md`](docs/LANGUAGE_COMPLETION_MATRIX.md) — language/backend qualification status
- [`docs/TEST_PROFILES.md`](docs/TEST_PROFILES.md) — test/build profiles
- [`docs/RELEASE_NOTES.md`](docs/RELEASE_NOTES.md) — version history

The example corpus has its own short guide in [`examples/README.md`](examples/README.md).

## Project status

**0.9.85 is the accepted GREEN implementation baseline; 0.9.86 is the public GitHub preparation release.** The current work is focused on closing real gaps in already implemented semantics, strengthening runtime and generated-code behavior, expanding conformance coverage, and preparing the project for eventual release hardening.

The language itself has no planned feature-count phase. If a future format or use case exposes a genuine semantic gap, it should first be demonstrated with a minimal reproducible example and regression test before the language is extended.

## Contributing

Contributions are welcome, especially:

- conformance fixtures for real binary formats;
- generated-C++ qualification cases;
- runtime and error-handling hardening;
- tests that expose reproducible semantic gaps;
- documentation improvements;
- additional generated-language backends that preserve the canonical Plan semantics.

Please keep the core design principle in mind: **minimality and sufficiency**. Prefer one clear semantic mechanism over multiple overlapping mechanisms.

## License

EmbX is released under the MIT License. See [`LICENSE`](LICENSE).
