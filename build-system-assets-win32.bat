@echo off

call %~dp0config.bat

%TRAKTOR_HOME%build\win32\releaseshared\Traktor.Pipeline.App -s=Traktor.Pipeline.Win32
