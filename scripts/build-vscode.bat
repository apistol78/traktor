@echo off

set PLATFORM=%1
set CONFIG=%2

call %~dp0config.bat

if "%PLATFORM%" == "Windows" (
    call %~dp0config-vs-x64.bat
    pushd "%TRAKTOR_HOME%\build\win64"
    devenv "Traktor Win64.sln" /Build %CONFIG%
    popd
)

if "%PLATFORM%" == "Android" (
    call %~dp0config-android.bat
    pushd "%TRAKTOR_HOME%\build\android"
    %TRAKTOR_HOME%\3rdp\jom\jom /L /F "Extern Android.mak" %CONFIG%
    %TRAKTOR_HOME%\3rdp\jom\jom /L /F "Traktor Android.mak" %CONFIG%
    popd
)
