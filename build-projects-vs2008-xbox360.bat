@call %~dp0config.bat
@%TRAKTOR_HOME%\bin\win64\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2008-xbox360.xml %TRAKTOR_HOME%\TraktorXbox360.xms
