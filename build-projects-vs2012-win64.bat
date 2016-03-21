@call %~dp0config.bat
@%TRAKTOR_HOME%\bin\win32\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2012-win64.xml %TRAKTOR_HOME%\TraktorWin64.xms
@echo vs2012_x64> build\win64\version.txt
