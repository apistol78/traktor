@echo off

call %~dp0config.bat

:: \hack Setup paths to ABI specific binaries.
set FBX_SDK_LIBPATH=%FBX_SDK%\lib\vs2015\x86\release
set P4_SDK=%TRAKTOR_HOME%\3rdp\p4api-2016.1.1350954.BETA-vs2015_dyn_x86

:: Mandatory parameters.
set FBUILD_INCLUDE=$(TRAKTOR_HOME)\resources\build\configurations\fbuild-config-win32.bff

:: Optional parameters.
set FBUILD_MANIFEST=%TRAKTOR_HOME%\resources\build\windows\manifest-win32.xml

%SOLUTIONBUILDER% ^
	-f=fbuild ^
	-fastbuild-solution-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-solution.sb ^
	-fastbuild-project-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-project.sb ^
	%TRAKTOR_HOME%\resources\build\ExternWin32.xms

%SOLUTIONBUILDER% ^
	-f=fbuild ^
	-fastbuild-solution-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-solution.sb ^
	-fastbuild-project-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-project.sb ^
	%TRAKTOR_HOME%\resources\build\TraktorWin32.xms
