@echo off
call "%~dp0build_profile.cmd" "ALL" 1
exit /b %errorlevel%
