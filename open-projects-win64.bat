@echo off

call %~dp0config.bat
set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\win64

start build\win64\"Traktor Win64.sln"
