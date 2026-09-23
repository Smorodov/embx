@echo off
setlocal
chcp 65001 > nul
set "ROOT=%~dp0"
set "BUILD_DIR=%ROOT%build"
set "LOGFILE=%ROOT%build_protocol.txt"

> "%LOGFILE%" echo Build started at %date% %time%
cmake -S . -B "%BUILD_DIR%" -G "MinGW Makefiles" >> "%LOGFILE%" 2>&1
if errorlevel 1 (type "%LOGFILE%" & pause & exit /b 1)
cmake --build "%BUILD_DIR%" -- -j4 >> "%LOGFILE%" 2>&1
if errorlevel 1 (type "%LOGFILE%" & pause & exit /b 1)
call "%ROOT%run_examples.cmd" >> "%LOGFILE%" 2>&1
if errorlevel 1 (type "%LOGFILE%" & pause & exit /b 1)
ctest --test-dir "%BUILD_DIR%" --output-on-failure >> "%LOGFILE%" 2>&1
if errorlevel 1 (type "%LOGFILE%" & pause & exit /b 1)
echo Build and tests complete. See "%LOGFILE%" for the full log.
pause
