@echo off
call "%~dp0build_profile.cmd" "EXTENDED" 1
exit /b %errorlevel%
