@echo off

:: Source environment configuration.
call %~dp0config.bat
call %~dp0config-android.bat

:: Launch remote server.
start %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Remote.Server.App.exe %APPDATA%\TraktorRemoteServer

:: Launch editor.
start %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Editor.App.exe
