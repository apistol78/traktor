@echo off

call %~dp0config.bat
call %~dp0config-vsenv.bat %TRAKTOR_HOME%\build\win32\version.txt

set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\win32

start %TRAKTOR_HOME%\build\win32\"Traktor Win32.sln"
