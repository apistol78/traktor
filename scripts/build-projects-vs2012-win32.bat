@call %~dp0config.bat

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2012-win32.xml ^
	%TRAKTOR_HOME%\resources\build\ExternWin32.xms

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2012-win32.xml ^
	%TRAKTOR_HOME%\resources\build\TraktorWin32.xms

@echo vs2012_x86> %TRAKTOR_HOME%\build\win32\version.txt
