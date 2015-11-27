@echo off

call %~dp0config.bat
set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\pnacl

start build\pnacl\"Traktor PNaCl.sln"
