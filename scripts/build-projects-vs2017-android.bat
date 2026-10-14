@echo off

call %~dp0config.bat
call %~dp0config-android.bat

set SOLUTIONBUILDER=%TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.SolutionBuilder.App

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2017-android.xml ^
	%TRAKTOR_HOME%\resources\build\ExternAndroid.xms

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2017-android.xml ^
	%TRAKTOR_HOME%\resources\build\TraktorAndroid.xms

