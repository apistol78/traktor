@echo off

call %~dp0config.bat
set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\android

start %TRAKTOR_HOME%\build\android\"Traktor Android.sln"
