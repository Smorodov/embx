@echo off
call "%~dp0build_profile.cmd" "DEV" 1
exit /b %errorlevel%
