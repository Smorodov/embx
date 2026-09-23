# Lesson 10 — Callbacks and transforms

This lesson combines two existing EmbX extension points without adding new semantic machinery:

- **callbacks** are host/runtime hooks at explicit points in encode/decode execution;
- **`scale(factor)`** converts a physical numeric field between wire and logical values while preserving its physical layout.

## Source

`callbacks_transforms.embx` contains two small independent structs.

### Callback example

```embx
callback on_decode_event(value);
callback on_encode_event(value);

struct DecodeEvent big {
  value: u8;
  callback on_decode_event(value);
}

struct EncodeEvent big {
  value: u8;
  callback on_encode_event(value);
}
```

The callback declarations define the available hooks. The member uses are placed after `value`, so the argument is available when the hook executes.

Callbacks do not automatically add bytes. In this lesson the host callback only observes the value. The reference runtime requires a configured callback registry when a callback is reached. The lesson keeps decode and encode callbacks in separate structs because the reference runtime explicitly rejects an opposite-direction callback when executing a codec path.

### Transform example

```embx
struct Measurement big {
  temperature: i16 transform scale(0.1);
}
```

The wire value `253` decodes to the logical value `25.3`. Encoding logical `25.3` produces the original wire value `253`.

The transform does not change the field's physical size or offset: `i16` remains two bytes.

## Expected bytes

For `DecodeEvent` / `EncodeEvent` with `value = 0x2A`:

```text
2A
```

For `Measurement` with logical `temperature = 25.3`:

```text
00 FD
```

## What the test proves

`lesson_10_callbacks_transforms_test` checks:

1. the course source compiles into an executable Plan;
2. decode callbacks receive the decoded field value;
3. encode callbacks receive the supplied logical field value;
4. callbacks do not change the wire bytes in this example;
5. `scale(0.1)` decodes `00 FD` as `25.3`;
6. encoding `25.3` reproduces `00 FD`;
7. the transform preserves the physical two-byte layout.

The lesson deliberately reuses the existing callback registry and `scale` transform contracts. It does not introduce lesson-specific compiler or runtime behavior.

## Reproducibility

From the build directory:

```text
ctest --test-dir build -R lesson_10_callbacks_transforms --output-on-failure
```

This lesson is part of the accepted green course baseline. Local validation is included in the 73/73 CTest suite.
