@echo off

:: Setup our build environment.
call %~dp0../config.bat

pushd %TRAKTOR_HOME%\doc

:: Remove old documentation.
del /F /Q html

:: Rebuild entire documentation.
doxygen traktor.doxycfg

:: Copy generated documentation.
del /F /Q latest
mkdir latest
xcopy /C /I /R /E /Y html latest

popd
