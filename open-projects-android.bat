@echo off

call %~dp0config.bat

start build\android\"Traktor Android.sln"
