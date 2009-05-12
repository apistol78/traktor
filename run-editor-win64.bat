@echo off

call %~dp0config.bat

start %TRAKTOR_HOME%\build\win64\releaseshared\Traktor.Editor.App
