@call %~dp0config.bat
%SOLUTIONBUILDER% -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2015-win64.xml %TRAKTOR_HOME%\TraktorWin64.xms
@echo vs2015_x64> build\win64\version.txt
