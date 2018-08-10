@echo off

call %~dp0config.bat

set SH=%TRAKTOR_HOME%\bin\win64\ShadowLaunch

:: Launch remote server.
%SH% TraktorRemoteServer %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Remote.Server.App.exe %APPDATA%\TraktorRemoteServer

:: Launch editor.
%SH% TraktorEditor %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Editor.App.exe
