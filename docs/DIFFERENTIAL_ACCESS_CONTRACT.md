# Differential Access Conformance Contract

## Scope

This corpus qualifies generated C++ against the Reference Runtime for existing
Plan semantics covering virtual fields, field aliases, and scalar transforms.

The test does not introduce new language semantics and does not add a second
semantic or layout engine.

## Required equivalence

For each valid fixture:

1. Reference Runtime encodes a value.
2. Generated C++ encodes the same logical value.
3. The resulting bytes must be identical.
4. Generated C++ decodes the reference bytes to equivalent logical values.
5. Virtual fields and aliases must expose the same logical facts without adding
   wire bytes.
6. A scalar `scale` transform must preserve the physical wire representation
   while mapping between wire and logical values identically.

## Current corpus

- `AccessPacket`: virtual field, field alias, dynamic payload size.
- `TransformPacket`: scalar `scale(0.1)` transform.

## Architectural boundary

The Reference Runtime remains the semantic reference. Generated C++ is a
separate implementation of the same Plan semantics. Format-specific behavior
is not introduced into the language or Plan.
