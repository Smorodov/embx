# Status: accepted green in EmbX 0.9.62 — 88/88 CTest tests

# Differential Terminated-Sequence Conformance Contract

This stage qualifies the existing terminated-sequence semantics of the Reference Runtime
against generated C++ without introducing a new language or semantic mechanism.

The corpus uses the existing Lesson 12 source:
`course/12_terminated_sequences/terminated_sequences.embx`.

Required equivalence:

- Reference Runtime encode → bytes
- Generated C++ encode → identical bytes
- Reference Runtime decode → value
- Generated C++ decode → equivalent value
- transactional failure for a missing outer terminator

The corpus deliberately uses a terminated sequence of structured elements, where each element
contains its own terminated byte field. The outer terminator is recognized only between complete
elements.
