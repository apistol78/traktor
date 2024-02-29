@echo off

set PLATFORM=%1
set CONFIG=%2

call %~dp0config.bat
call %~dp0config-vs-x64.bat

echo Building %CONFIG%...
echo %TRAKTOR_HOME%

if "%PLATFORM%" == "Windows" (
	pushd "%TRAKTOR_HOME%\build\win64"
	msbuild -m "Traktor Win64.sln" /p:Configuration=%CONFIG% /p:Platform="x64"
	popd
)

if "%PLATFORM%" == "Android" (
	pushd "%TRAKTOR_HOME%\build\android"
	%TRAKTOR_HOME%\3rdp\jom-windows\jom /L /F "Extern Android.mak" %CONFIG%
	%TRAKTOR_HOME%\3rdp\jom-windows\jom /L /F "Traktor Android.mak" %CONFIG%
	popd
)
