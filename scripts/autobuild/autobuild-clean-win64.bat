@echo off

call "%~dp0..\config.bat"

pushd %TRAKTOR_HOME%
rmdir /Q /S "build\win64" 2> nul
rmdir /Q /S "bin\latest\win64" 2> nul
popd
