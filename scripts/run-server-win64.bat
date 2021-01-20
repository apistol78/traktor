@echo off

:: Source environment configuration.
call %~dp0config.bat

:: Launch remote server.
start %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Run.App.exe $(TRAKTOR_HOME)\scripts\remote\Server.run %APPDATA%\Traktor\Editor\Remote
