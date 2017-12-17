@call %~dp0config.bat
%SOLUTIONBUILDER% -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2012-ps3.xml %TRAKTOR_HOME%\TraktorPs3.xms
%SOLUTIONBUILDER% -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2012-ps3-spu.xml %TRAKTOR_HOME%\TraktorPs3-Spu.xms
