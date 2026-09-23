param(
    [string]$MidiFile = (Join-Path $PSScriptRoot 'midi_demo.mid')
)

Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class EmbXMci {
    [DllImport("winmm.dll", CharSet = CharSet.Unicode)]
    public static extern int mciSendString(string command, System.Text.StringBuilder returnValue, int returnLength, IntPtr callback);
}
'@

if (-not (Test-Path -LiteralPath $MidiFile)) {
    Write-Error "MIDI file not found: $MidiFile"
    exit 2
}

$alias = 'embx_midi'
$path = (Resolve-Path -LiteralPath $MidiFile).Path
$escaped = $path.Replace('"', '""')
$rc = [EmbXMci]::mciSendString("open `"$escaped`" type sequencer alias $alias", $null, 0, [IntPtr]::Zero)
if ($rc -ne 0) {
    Write-Error "Windows MCI could not open the MIDI file (error $rc)."
    exit $rc
}

try {
    $rc = [EmbXMci]::mciSendString("play $alias wait", $null, 0, [IntPtr]::Zero)
    if ($rc -ne 0) {
        Write-Error "Windows MCI could not play the MIDI file (error $rc)."
        exit $rc
    }
}
finally {
    [void][EmbXMci]::mciSendString("close $alias", $null, 0, [IntPtr]::Zero)
}
