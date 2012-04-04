@echo off

call %~dp0config.bat

start %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Editor.App
