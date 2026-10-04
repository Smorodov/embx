@echo off
setlocal EnableDelayedExpansion
chcp 65001 > nul

set "ROOT=%~dp0"
set "BUILD_DIR=%ROOT%build"
set "LOGFILE=%ROOT%build_protocol.txt"
set "PROFILE=%~1"
set "RUN_EXAMPLES=%~2"
set "PIPE=|"
set "LABEL_REGEX="

if /I "%PROFILE%"=="DEV" set "LABEL_REGEX=CORE!PIPE!ACTIVE"
if /I "%PROFILE%"=="EXTENDED" set "LABEL_REGEX=CORE!PIPE!ACTIVE!PIPE!EXTENDED"
if /I "%PROFILE%"=="AUDIT" set "LABEL_REGEX=CORE!PIPE!ACTIVE!PIPE!AUDIT"
if /I "%PROFILE%"=="ALL" set "LABEL_REGEX="

if /I not "%PROFILE%"=="DEV" if /I not "%PROFILE%"=="EXTENDED" if /I not "%PROFILE%"=="AUDIT" if /I not "%PROFILE%"=="ALL" (
    echo ERROR: unknown test profile "%PROFILE%".
    echo Use DEV, EXTENDED, AUDIT or ALL.
    exit /b 2
)

> "%LOGFILE%" echo Build started at %date% %time%
>> "%LOGFILE%" echo Test profile: %PROFILE%

cmake -S . -B "%BUILD_DIR%" -G "MinGW Makefiles" >> "%LOGFILE%" 2>&1
if errorlevel 1 (type "%LOGFILE%" & pause & exit /b 1)

cmake --build "%BUILD_DIR%" -- -j4 >> "%LOGFILE%" 2>&1
if errorlevel 1 (type "%LOGFILE%" & pause & exit /b 1)

if "%RUN_EXAMPLES%"=="1" (
    call "%ROOT%run_examples.cmd" >> "%LOGFILE%" 2>&1
    if errorlevel 1 (type "%LOGFILE%" & pause & exit /b 1)
)

if /I "%PROFILE%"=="ALL" (
    ctest --test-dir "%BUILD_DIR%" --output-on-failure >> "%LOGFILE%" 2>&1
) else (
    ctest --test-dir "%BUILD_DIR%" -L "!LABEL_REGEX!" --output-on-failure >> "%LOGFILE%" 2>&1
)
if errorlevel 1 (type "%LOGFILE%" & pause & exit /b 1)

echo.
echo === BUILD PROFILE %PROFILE% PASSED ===
echo.
pause
exit /b 0
