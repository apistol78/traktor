@echo off

call %~dp0config.bat

start %TRAKTOR_HOME%\build\android\"Traktor Android.sln"
