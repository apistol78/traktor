@call %~dp0config.bat
%SOLUTIONBUILDER% -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2015-ps4.xml %TRAKTOR_HOME%\TraktorPs4.xms
