@echo off

call %~dp0config.bat

set SOLUTIONBUILDER=%TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.SolutionBuilder.App

:: \hack Setup paths to ABI specific binaries.
set FBX_SDK_LIBPATH=%FBX_SDK%\lib\vs2015\x64\release
set P4_SDK=%TRAKTOR_HOME%\3rdp\p4api-2016.1.1350954.BETA-vs2015_dyn_x64

:: Optional parameters.
set NINJA_MANIFEST=%TRAKTOR_HOME%\resources\build\windows\manifest-win64.xml

:: Mandatory parameters.
set NINJA_INCLUDE=%TRAKTOR_HOME%\resources\build\configurations\ninja-config-win64.ninja

::%SOLUTIONBUILDER% ^
::	-f=ninja ^
::	-ninja-solution-template=%TRAKTOR_HOME%\resources\build\configurations\ninja-solution.sb ^
::	-ninja-project-template=%TRAKTOR_HOME%\resources\build\configurations\ninja-project.sb ^
::	%TRAKTOR_HOME%\resources\build\ExternWin64.xms

%SOLUTIONBUILDER% ^
	-f=ninja ^
	-ninja-solution-template=%TRAKTOR_HOME%\resources\build\configurations\ninja-solution.sb ^
	-ninja-project-template=%TRAKTOR_HOME%\resources\build\configurations\ninja-project.sb ^
	%TRAKTOR_HOME%\resources\build\TraktorWin64.xms
