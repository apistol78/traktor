@echo off

call "%~dp0..\config.bat"

pushd %TRAKTOR_HOME%
rmdir /Q /S "build" 2> nul
popd
