@echo off

call %~dp0config.bat

:: \hack Setup paths to ABI specific binaries.
set P4_DEBUG_SDK=%TRAKTOR_HOME%\3rdp\p4api-2020.1.1953492-vs2017_dyn_vsdebug
set P4_RELEASE_SDK=%TRAKTOR_HOME%\3rdp\p4api-2020.1.1953492-vs2017_dyn

:: Mandatory parameters.
set MAKE_OS=windows
set MAKE_DIALECT=nmake
set MAKE_FORMAT=%TRAKTOR_HOME%\resources\build\configurations\make-format-vc.sbm
set MAKE_INCLUDE=$(TRAKTOR_HOME)\resources\build\configurations\make-config-win64.inc

:: Optional parameters.
set MAKE_MANIFEST=%TRAKTOR_HOME%\resources\build\windows\manifest-win64.xml

%SOLUTIONBUILDER% ^
	-f=make2 ^
	-make-solution-template=%TRAKTOR_HOME%\resources\build\configurations\make-solution.sbm ^
	-make-project-template=%TRAKTOR_HOME%\resources\build\configurations\make-project.sbm ^
	%TRAKTOR_HOME%\resources\build\ExternWin64.xms

%SOLUTIONBUILDER% ^
	-f=make2 ^
	-make-solution-template=%TRAKTOR_HOME%\resources\build\configurations\make-solution.sbm ^
	-make-project-template=%TRAKTOR_HOME%\resources\build\configurations\make-project.sbm ^
	%TRAKTOR_HOME%\resources\build\TraktorWin64.xms
