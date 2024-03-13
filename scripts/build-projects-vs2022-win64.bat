@echo off

call %~dp0config.bat

:: \hack Setup paths to ABI specific binaries.
set P4_DEBUG_SDK=%TRAKTOR_HOME%\3rdp\p4api-2020.1.1953492-vs2017_dyn_vsdebug
set P4_RELEASE_SDK=%TRAKTOR_HOME%\3rdp\p4api-2020.1.1953492-vs2017_dyn
set OPENSSL_SDK=%TRAKTOR_HOME%\3rdp\openssl-1.0.2u-x64_86

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2022-win64.xml ^
	%TRAKTOR_HOME%\resources\build\ExternWin64.xms

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2022-win64.xml ^
	%TRAKTOR_HOME%\resources\build\TraktorWin64.xms
