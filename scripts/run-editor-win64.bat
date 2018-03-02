@echo off

call %~dp0config.bat

:: Launch remote server.
start %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Remote.Server.App %APPDATA%\TraktorRemoteServer

:: Launch editor.
start %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Editor.App
