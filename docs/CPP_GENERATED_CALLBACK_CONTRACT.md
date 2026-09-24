# EmbX C++ Generated Callback Contract

## Scope

The PASS14 implementation closes the callback boundary in the C++ generated backend. The semantic callback model, AST, IR, Plan and reference runtime registry remain unchanged.

The generated backend exposes one explicit external callback registry:

- `embx_generated_detail::Callbacks`
- separate decode and encode registrations
- string callback names are resolved only at this external boundary
- callback arguments are represented by `CallbackValue`
- runtime module parameters are passed through the existing generated parameter object

No second resolver, expression evaluator, IR, plugin loader or FFI layer is introduced.

## Generated callback values

`CallbackValue` is the minimal scalar transport type:

- `std::int64_t`
- `std::uint64_t`
- `double`
- `bool`

Callback arguments are emitted from the existing `Plan::Expr` representation. The generated backend does not reinterpret or re-evaluate the expression.

## Decode

A generated decode callback receives the generated `Reader`, module runtime parameters when present, evaluated callback arguments, and an error string.

The callback boundary is transactional: Reader position and error state are restored when the callback fails or throws.

## Encode

A generated encode callback receives the generated `Writer`, module runtime parameters when present, evaluated callback arguments, and an error string.

The callback boundary is transactional: output size and Writer position are restored when the callback fails or throws. Advancing beyond the available output is rejected.

## Direction

`on_decode*` callbacks are emitted only in decode paths. `on_encode*` callbacks are emitted only in encode paths. A single struct may contain both directions; each codec path ignores callbacks belonging to the opposite direction. An unspecified callback direction remains a generator error.

## API compatibility

The generated `decode__*` and `encode__*` functions retain their existing call shape. The callback registry is an optional final parameter, so generated code without callbacks requires no registry.

## Deliberate boundary

PASS14 does not introduce dynamic plugin loading, language FFI, Rust/Python backends, or another callback dispatch mechanism. Those concerns remain outside the C++ release scope.
