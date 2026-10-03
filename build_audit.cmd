@echo off
call "%~dp0build_profile.cmd" "AUDIT" 1
exit /b %errorlevel%
