from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent
POSITIVE = {
    'valid_format0.mid', 'valid_format1.mid', 'valid_format2.mid',
    'valid_multitrack.mid', 'valid_tempo.mid', 'valid_timesig.mid',
    'valid_meta_events.mid', 'valid_sysex.mid', 'valid_channel_mode.mid',
    'valid_system_common.mid', 'valid_system_realtime.mid',
}
NEGATIVE = {
    'invalid_empty.mid', 'invalid_header.mid', 'invalid_format.mid',
    'invalid_track_chunk.mid', 'invalid_truncated.mid', 'invalid_no_eot.mid',
    'invalid_note_range.mid', 'invalid_running_status.mid',
    'invalid_meta_event.mid',
}

def u16(b, p): return (b[p] << 8) | b[p + 1]
def u32(b, p): return (b[p] << 24) | (b[p + 1] << 16) | (b[p + 2] << 8) | b[p + 3]

def vlq(b, p, end):
    value = 0
    for _ in range(4):
        if p >= end: raise ValueError('truncated VLQ')
        x = b[p]; p += 1
        value = (value << 7) | (x & 0x7f)
        if not x & 0x80: return value, p
    raise ValueError('VLQ too long')

def valid(path):
    b = path.read_bytes()
    if len(b) < 14 or b[:4] != b'MThd' or u32(b, 4) != 6:
        return False
    fmt, tracks, division = u16(b, 8), u16(b, 10), u16(b, 12)
    if fmt not in (0, 1, 2) or tracks == 0 or division == 0:
        return False
    p = 14
    if fmt == 0 and tracks != 1: return False
    for _ in range(tracks):
        if p + 8 > len(b) or b[p:p+4] != b'MTrk': return False
        length = u32(b, p + 4); start = p + 8; end = start + length
        if end > len(b): return False
        q = start; running = None; eot = False
        while q < end:
            _, q = vlq(b, q, end)
            if q >= end: return False
            status = b[q]
            if status < 0x80:
                if running is None or running >= 0xF0: return False
                status = running
            else:
                q += 1
                if 0x80 <= status < 0xF0: running = status
                elif status < 0xF8: running = None
            if status == 0xFF:
                if q >= end: return False
                meta = b[q]; q += 1
                n, q = vlq(b, q, end)
                if q + n > end: return False
                q += n
                if meta == 0x2F:
                    if n != 0 or q != end: return False
                    eot = True
                elif meta not in (0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x20, 0x21, 0x51, 0x54, 0x58, 0x59, 0x7F):
                    return False
            elif status in (0xF0, 0xF7):
                n, q = vlq(b, q, end)
                if q + n > end: return False
                q += n
            elif 0x80 <= status <= 0xEF:
                n = 1 if (status & 0xE0) == 0xC0 else 2
                if q + n > end: return False
                if status & 0xF0 in (0x80, 0x90):
                    note = b[q]
                    if note > 127: return False
                q += n
            elif 0xF1 <= status <= 0xF3:
                n = {0xF1:1, 0xF2:2, 0xF3:1}[status]
                if q + n > end: return False
                q += n
            elif status == 0xF6:
                pass
            elif 0xF8 <= status <= 0xFE:
                pass
            else:
                return False
        if not eot: return False
        p = end
    return p == len(b)

def main():
    failures = []
    for name in sorted(POSITIVE | NEGATIVE):
        path = ROOT / name
        expected = name in POSITIVE
        actual = valid(path)
        if actual != expected:
            failures.append(f'{name}: expected {expected}, got {actual}')
        else:
            print(f'PASS {name}')
    if failures:
        for x in failures: print('FAIL', x)
        return 1
    print(f'All {len(POSITIVE) + len(NEGATIVE)} MIDI corpus cases passed.')
    return 0

if __name__ == '__main__': sys.exit(main())
