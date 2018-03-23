@echo off

call %~dp0config.bat

:: Check prequisite.
if "%ANDROID_NDK_ROOT%" == "" (
	echo ANDROID_NDK_ROOT must be set!
	exit /b
)

:: Mandatory parameters.
set MAKE_OS=windows
set MAKE_DIALECT=nmake
set MAKE_FORMAT=%TRAKTOR_HOME%\resources\build\configurations\make-format.sbm
set MAKE_INCLUDE=$(TRAKTOR_HOME)\resources\build\configurations\make-config-android-win.inc

:: Used by default format.
set MAKE_TOOLCHAIN=gcc
set MAKE_OBJECT_FILE=%%s.o
set MAKE_STATIC_LIBRARY_FILE=lib%%s.a
set MAKE_SHARED_LIBRARY_FILE=lib%%s.so
set MAKE_EXECUTABLE_FILE=%%s

%SOLUTIONBUILDER% ^
	-f=make2 ^
	-make-solution-template=%TRAKTOR_HOME%\resources\build\configurations\make-solution.sbm ^
	-make-project-template=%TRAKTOR_HOME%\resources\build\configurations\make-project.sbm ^
	%TRAKTOR_HOME%\resources\build\ExternAndroid.xms

%SOLUTIONBUILDER% ^
	-f=make2 ^
	-make-solution-template=%TRAKTOR_HOME%\resources\build\configurations\make-solution.sbm ^
	-make-project-template=%TRAKTOR_HOME%\resources\build\configurations\make-project.sbm ^
	%TRAKTOR_HOME%\resources\build\TraktorAndroid.xms
