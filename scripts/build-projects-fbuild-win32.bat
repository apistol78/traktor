@echo off

call %~dp0config.bat

:: \hack Setup paths to ABI specific binaries.
set FBX_SDK_LIBPATH=%FBX_SDK%\lib\vs2015\x86\release
set P4_SDK=%TRAKTOR_HOME%\3rdp\p4api-2016.1.1350954.BETA-vs2015_dyn_x86

%SOLUTIONBUILDER% ^
	-f=fbuild ^
	-fastbuild-solution-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-solution-win32.sb ^
	-fastbuild-project-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-project-win32.sb ^
	%TRAKTOR_HOME%\resources\build\ExternWin32.xms

%SOLUTIONBUILDER% ^
	-f=fbuild ^
	-fastbuild-solution-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-solution-win32.sb ^
	-fastbuild-project-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-project-win32.sb ^
	%TRAKTOR_HOME%\resources\build\TraktorWin32.xms
