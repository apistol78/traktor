@echo off

call %~dp0config.bat

echo %SHADOWLAUNCH%

:: Launch remote server.
%TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Store.Server.App.exe -listen-port=80 -store-path=%TRAKTOR_HOME%\data\Store
