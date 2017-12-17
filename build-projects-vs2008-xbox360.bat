@call %~dp0config.bat
%SOLUTIONBUILDER% ^
	-f=msvc ^
	-p=$(TRAKTOR_HOME)\bin\msvc-2008-xbox360.xml ^
	%TRAKTOR_HOME%\TraktorXbox360.xms
