@call %~dp0config.bat
%SOLUTIONBUILDER% -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2015-android.xml %TRAKTOR_HOME%\TraktorAndroid.xms

