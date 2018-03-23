@echo off

call "%~dp0..\config.bat"

pushd %TRAKTOR_HOME%
rmdir /Q /S "build\android" 2> nul
rmdir /Q /S "bin\latest\android" 2> nul
popd
