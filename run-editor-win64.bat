@echo off

call %~dp0config.bat

:: Launch remote server.
set SCRATCH_PATH=%APPDATA%\TraktorRemoteServer
if not exist %SCRATCH_PATH% ( mkdir %SCRATCH_PATH% )

start %TRAKTOR_HOME%\bin\win32\RemoteServer %SCRATCH_PATH%

:: Launch editor.
start %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Editor.App
