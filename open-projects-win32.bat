@echo off

call %~dp0config.bat
call %~dp0config-vsenv.bat build\win32\version.txt

set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\win32

start build\win32\"Traktor Win32.sln"
