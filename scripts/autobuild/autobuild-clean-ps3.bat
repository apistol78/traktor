@echo off

call "%~dp0..\config.bat"

pushd %TRAKTOR_HOME%
rmdir /Q /S "build\ps3" 2> nul
rmdir /Q /S "bin\latest\ps3" 2> nul
popd
