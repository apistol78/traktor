@echo off

call %~dp0config.bat
call %~dp0config-android.bat
call %~dp0config-emscripten.bat

start %TRAKTOR_HOME%\build\win64\"Traktor Win64.sln"
