@echo off

set CONFIG=%*

call %~dp0config.bat
call %~dp0config-vs-x64.bat

pushd "%TRAKTOR_HOME%\build\win64"
devenv "Traktor Win64.sln" /Build %CONFIG%
popd
