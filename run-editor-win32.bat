@echo off

call %~dp0config.bat

:: Launch remote server.
%TRAKTOR_HOME%\bin\win32\ShadowLaunch "{E569D4C9-3D5C-4621-812A-D6E56E591858}" "%TRAKTOR_HOME%\bin\latest\win32\releaseshared\Traktor.Remote.Server.App.exe" "%APPDATA%\TraktorRemoteServer"

:: Launch editor.
%TRAKTOR_HOME%\bin\win32\ShadowLaunch "{4B172047-9960-4B19-A974-892751CA50CC}" "%TRAKTOR_HOME%\bin\latest\win32\releaseshared\Traktor.Editor.App.exe"
