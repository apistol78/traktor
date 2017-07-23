@call %~dp0config.bat
@%TRAKTOR_HOME%\bin\win64\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2012-win32.xml %TRAKTOR_HOME%\TraktorWin32.xms
@echo vs2012_x86> build\win32\version.txt
