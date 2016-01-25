@echo off

call %~dp0config.bat

:: Launch remote server.
start %TRAKTOR_HOME%\bin\latest\win32\releaseshared\Traktor.Remote.Server.App %APPDATA%\TraktorRemoteServer
