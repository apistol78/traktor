@echo off

call %~dp0../../config.bat

start build\win32\"MigrateDb Win32.sln"
