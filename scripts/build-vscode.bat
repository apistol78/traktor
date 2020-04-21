@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

call %~dp0config.bat
call %~dp0config-vs-x64.bat

set FBUILD=%TRAKTOR_HOME%\3rdp\FASTBuild-Windows-x64-v0.95\FBuild.exe

pushd "%TRAKTOR_HOME%\build\win64"
%FBUILD% -config "Extern Win64.bff" -noprogress -summary -cache %CONFIG%
%FBUILD% -config "Traktor Win64.bff" -noprogress -summary -cache %CONFIG%
popd
