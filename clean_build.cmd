@echo off
setlocal
chcp 65001 > nul
set "ROOT=%~dp0"
set "BUILD_DIR=%ROOT%build"

if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
if exist "%BUILD_DIR%" (
  echo Failed to remove build directory: "%BUILD_DIR%"
  exit /b 1
)
call "%ROOT%build.cmd"
exit /b %errorlevel%
