@call %~dp0config.bat

%SOLUTIONBUILDER% -f=msvc -i -p=$(TRAKTOR_HOME)\bin\msvc-2017-android.xml %TRAKTOR_HOME%\ExternAndroid.xms
%SOLUTIONBUILDER% -f=msvc -i -p=$(TRAKTOR_HOME)\bin\msvc-2017-android.xml %TRAKTOR_HOME%\TraktorAndroid.xms

