@call %~dp0config.bat

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2012-win64.xml ^
	%TRAKTOR_HOME%\resources\build\ExternWin64.xms

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2012-win64.xml ^
	%TRAKTOR_HOME%\resources\build\TraktorWin64.xms

@echo vs2012_x64> %TRAKTOR_HOME%\build\win64\version.txt
