@echo off

call %~dp0../../config.bat

start build\win64\FlashRemoteDebugger.sln
