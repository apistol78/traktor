@call %~dp0config.bat
@%TRAKTOR_HOME%\bin\win64\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2012-ps4.xml %TRAKTOR_HOME%\TraktorPs4.xms
