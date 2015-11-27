@echo off

call %~dp0config.bat
set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\xbox360

start build\xbox360\"Traktor Xbox360.sln"
