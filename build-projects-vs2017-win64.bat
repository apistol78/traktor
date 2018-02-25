@call %~dp0config.bat

%SOLUTIONBUILDER% -f=msvc -i -p=$(TRAKTOR_HOME)\bin\msvc-2017-win64.xml %TRAKTOR_HOME%\ExternWin64.xms
%SOLUTIONBUILDER% -f=msvc -i -p=$(TRAKTOR_HOME)\bin\msvc-2017-win64.xml %TRAKTOR_HOME%\TraktorWin64.xms

@echo vs2017_x64> build\win64\version.txt
