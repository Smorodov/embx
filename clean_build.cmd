@echo off
setlocal
set "ROOT=%~dp0"
set "BUILD_DIR=%ROOT%build"

if exist "%BUILD_DIR%" (
    rmdir /s /q "%BUILD_DIR%"
    if errorlevel 1 (
        echo ERROR: failed to remove "%BUILD_DIR%"
        exit /b 1
    )
)

call "%ROOT%build.cmd"
exit /b %errorlevel%
