@echo off

call %~dp0config.bat

%TRAKTOR_HOME%build\win32\releaseshared\Traktor.Pipeline.App -s=Traktor.Pipeline.Xbox360 {666412C3-5A4E-1B47-87D3-BC0165D2DC26}
