@echo off

call %~dp0config.bat

start %TRAKTOR_HOME%\build\win32\releaseshared\Traktor.Editor.App
