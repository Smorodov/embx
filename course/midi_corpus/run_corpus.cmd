@echo off
setlocal
where python.exe >nul 2>nul
if errorlevel 1 (
  echo Python 3 is required for the MIDI corpus checker.
  exit /b 2
)
python.exe "%~dp0check_corpus.py"
exit /b %errorlevel%
