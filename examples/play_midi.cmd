@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0play_midi.ps1" "%~dp0midi_demo.mid"
if errorlevel 1 (
  echo MIDI playback failed. Check that Windows has a MIDI output device/synthesizer configured.
  exit /b 1
)
echo MIDI playback completed successfully.
