@echo off

call %~dp0config.bat

:: \hack Setup paths to ABI specific binaries.
set FBX_SDK_LIBPATH=%FBX_SDK%\lib\vs2015\x64\release
set P4_SDK=%TRAKTOR_HOME%\3rdp\p4api-2018.1.1637071-vs2017_static_x64

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2017-win64.xml ^
	%TRAKTOR_HOME%\resources\build\ExternWin64.xms

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2017-win64.xml ^
	%TRAKTOR_HOME%\resources\build\TraktorWin64.xms
