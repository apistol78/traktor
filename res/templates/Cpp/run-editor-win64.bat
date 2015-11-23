@echo off

call %~dp0config.bat

set PATH=%GAME_HOME%/bin/latest/win64/releaseshared;%PATH%

call %TRAKTOR_HOME%/run-server-win32.bat
start Traktor.Editor.App.exe
