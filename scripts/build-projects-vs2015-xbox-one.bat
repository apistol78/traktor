@echo off

call %~dp0config.bat

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2015-xbox-one.xml ^
	%TRAKTOR_HOME%\resources\build\ExternXboxOne.xms

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2015-xbox-one.xml ^
	%TRAKTOR_HOME%\resources\build\TraktorXboxOne.xms
