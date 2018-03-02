@echo off

call %~dp0config.bat
set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\xbox-one

start %TRAKTOR_HOME%\build\xbox-one\"Traktor Xbox One.sln"
