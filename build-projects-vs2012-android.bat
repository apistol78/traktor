@echo off

call %~dp0config.bat

mkdir %TRAKTOR_HOME%\build

%TRAKTOR_HOME%\bin\win32\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2012-android.xml %TRAKTOR_HOME%\TraktorAndroid.xms
