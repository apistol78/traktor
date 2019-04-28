@echo off

call %~dp0config.bat

:: Launch remote server.
start %SHADOWLAUNCH% %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Store.Server.App.exe
