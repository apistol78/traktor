@echo off

call %~dp0config.bat

start %TRAKTOR_HOME%\build\win32\releaseshared\Traktor.Database.Remote.Server.App.exe -c=Traktor.Database.Remote.Server.config

