@echo off

call %~dp0config.bat

set SCRATCH_PATH=%APPDATA%\TraktorRemoteServer
if not exist %SCRATCH_PATH% ( mkdir %SCRATCH_PATH% )

start %TRAKTOR_HOME%/bin/RemoteServer %SCRATCH_PATH%
