@call %~dp0config.bat
%SOLUTIONBUILDER% -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2012-ps4.xml %TRAKTOR_HOME%\TraktorPs4.xms
