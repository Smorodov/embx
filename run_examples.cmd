@echo off
setlocal enabledelayedexpansion
if not exist build\embx.exe (
  echo Build first: build.cmd
  exit /b 2
)
set COUNT=0
for %%F in (examples\*.embx) do (
  set /a COUNT+=1
  echo === %%F ===
  build\embx.exe "%%F" --dump-ast
  if errorlevel 1 exit /b 1
)
if "!COUNT!"=="0" (
  echo No canonical examples found.
  exit /b 1
)
echo All examples passed.
