@echo off

call %~dp0config.bat
set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\ps4

start build\ps4\"Traktor Ps4.sln"

