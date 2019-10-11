@echo off

:: Source environment configuration.
call %~dp0config.bat

:: Launch remote server.
start %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Remote.Server.App.exe %APPDATA%\TraktorRemoteServer
