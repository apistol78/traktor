@call %~dp0config.bat

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2012-ps3.xml ^
	%TRAKTOR_HOME%\resources\build\ExternPs3.xms

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2012-ps3.xml ^
	%TRAKTOR_HOME%\resources\build\TraktorPs3.xms

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2012-ps3-spu.xml ^
	%TRAKTOR_HOME%\resources\build\TraktorPs3-Spu.xms
