@echo off

rem Setup VC environment variables.
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"

rem Setup our build environment.
call %~dp0config.bat

devenv %*
