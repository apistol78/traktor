@echo off

call %~dp0config.bat

:: \hack Setup paths to ABI specific binaries.
set FBX_SDK_LIBPATH=%FBX_SDK%\lib\vs2017\x64\release
set P4_DEBUG_SDK=%TRAKTOR_HOME%\3rdp\p4api-2020.1.1953492-vs2017_dyn_vsdebug
set P4_RELEASE_SDK=%TRAKTOR_HOME%\3rdp\p4api-2020.1.1953492-vs2017_dyn

:: Optional parameters.
set FBUILD_MANIFEST=%TRAKTOR_HOME%\resources\build\windows\manifest-win64.xml

:: Mandatory parameters.
set FBUILD_INCLUDE=$(TRAKTOR_HOME)\resources\build\configurations\fbuild-config-win64-no-debug.bff

%SOLUTIONBUILDER% ^
	-f=fbuild ^
	-fastbuild-solution-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-solution.sb ^
	-fastbuild-project-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-project.sb ^
	%TRAKTOR_HOME%\resources\build\ExternWin64.xms

:: Mandatory parameters.
set FBUILD_INCLUDE=$(TRAKTOR_HOME)\resources\build\configurations\fbuild-config-win64.bff

%SOLUTIONBUILDER% ^
	-f=fbuild ^
	-fastbuild-solution-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-solution.sb ^
	-fastbuild-project-template=%TRAKTOR_HOME%\resources\build\configurations\fbuild-project.sb ^
	%TRAKTOR_HOME%\resources\build\TraktorWin64.xms
