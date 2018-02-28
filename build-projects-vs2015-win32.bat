@call %~dp0config.bat

%SOLUTIONBUILDER% -f=msvc -i -p=$(TRAKTOR_HOME)\bin\msvc-2015-win32.xml %TRAKTOR_HOME%\ExternWin32.xms
%SOLUTIONBUILDER% -f=msvc -i -p=$(TRAKTOR_HOME)\bin\msvc-2015-win32.xml %TRAKTOR_HOME%\TraktorWin32.xms

@echo vs2015_x86> build\win32\version.txt
