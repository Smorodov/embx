# MIDI conformance corpus

This corpus is derived from the real MIDI fixtures supplied for the EmbX MIDI work.
It is deliberately split into small positive and negative cases so the course can
teach both accepted SMF structure and rejection of malformed input.

## Positive cases

- `valid_format0.mid`
- `valid_format1.mid`
- `valid_format2.mid`
- `valid_multitrack.mid`
- `valid_tempo.mid`
- `valid_timesig.mid`
- `valid_meta_events.mid`
- `valid_sysex.mid`
- `valid_channel_mode.mid`
- `valid_system_common.mid`
- `valid_system_realtime.mid`

## Negative cases

- `invalid_empty.mid`
- `invalid_header.mid`
- `invalid_format.mid`
- `invalid_track_chunk.mid`
- `invalid_truncated.mid`
- `invalid_no_eot.mid`
- `invalid_note_range.mid`
- `invalid_running_status.mid`
- `invalid_meta_event.mid`

The corpus is a teaching and conformance fixture. It is not part of the EmbX
semantic core and does not introduce MIDI-specific constructs into the language.
