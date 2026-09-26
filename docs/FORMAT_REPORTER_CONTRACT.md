# EmbX Format Reporter Contract

Status: accepted green / 0.9.58

The Format Reporter is a deterministic presentation layer over canonical Plan/Reflection facts. It may use the existing `plan::LayoutGraph` for dependency facts, but must not implement a second semantic, dependency or layout model.

It reports, where canonically established: declaration counts, structure layout class, minimum/maximum size, byte order, dynamic fields, aliases/virtuals, dependency counts, runtime parameters, and known field offsets/sizes. Facts not established by Plan/Reflection are reported as `unknown`; the reporter never infers them.

Output is stable plain text, four-space indentation, one fact per line and exactly one final newline. It is not an EmbX source format.

Acceptance requires deterministic output, representative exact/bounded/unbounded and dynamic cases, dependency reporting through the existing graph, no duplicated semantic/layout calculation, and a green full test/example gate. The 0.9.58 acceptance gate is 82/82 CTest tests (100%).
