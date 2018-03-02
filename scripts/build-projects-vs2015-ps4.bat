@call %~dp0config.bat

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2015-ps4.xml ^
	%TRAKTOR_HOME%\resources\build\ExternPs4.xms

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2015-ps4.xml ^
	%TRAKTOR_HOME%\resources\build\TraktorPs4.xms
