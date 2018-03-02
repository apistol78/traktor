@echo off

call %~dp0config.bat
set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\xbox360

start %TRAKTOR_HOME%\build\xbox360\"Traktor Xbox360.sln"
