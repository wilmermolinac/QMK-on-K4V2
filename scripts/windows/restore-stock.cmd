@echo off
rem Runs restore-stock.ps1 with Windows PowerShell. Works from scripts\windows\ or from
rem the root of the release kit. You can also drag a file onto this .cmd.
setlocal
set "PS1=%~dp0restore-stock.ps1"
if not exist "%PS1%" set "PS1=%~dp0scripts\windows\restore-stock.ps1"
powershell.exe -NoProfile -Sta -ExecutionPolicy Bypass -File "%PS1%" %*
set "RC=%ERRORLEVEL%"
echo.
pause
exit /b %RC%
