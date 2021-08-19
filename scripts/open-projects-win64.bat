@echo off

call %~dp0config.bat
call %~dp0config-android.bat

start %TRAKTOR_HOME%\build\win64\"Traktor Win64.sln"
