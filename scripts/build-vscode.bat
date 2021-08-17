@echo off

set PLATFORM=%1
set CONFIG=%2

call %~dp0config.bat

if "%PLATFORM%" == "Windows" (
    call %~dp0config-vs-x64.bat
    pushd "%TRAKTOR_HOME%\build\win64"
    %FBUILD% -config "Extern Win64.bff" -noprogress -cache %CONFIG%
    %FBUILD% -config "Traktor Win64.bff" -noprogress -cache %CONFIG%
    popd
)

if "%PLATFORM%" == "Android" (
    call %~dp0config-android.bat
    pushd "%TRAKTOR_HOME%\build\android"
    %TRAKTOR_HOME%\3rdp\jom\jom /L /F "Extern Android.mak" %CONFIG%
    %TRAKTOR_HOME%\3rdp\jom\jom /L /F "Traktor Android.mak" %CONFIG%
    popd
)

if "%PLATFORM%" == "PS3" (
    call %~dp0config-vs-x64.bat
    pushd "%TRAKTOR_HOME%\build\ps3"
    %TRAKTOR_HOME%\3rdp\jom\jom /L /F "Extern Ps3.mak" %CONFIG%
    %TRAKTOR_HOME%\3rdp\jom\jom /L /F "Traktor Ps3.mak" %CONFIG%
    popd
    pushd "%TRAKTOR_HOME%\build\ps3-spu"
    %TRAKTOR_HOME%\3rdp\jom\jom /L /F "Traktor Ps3 SPU.mak" %CONFIG%
    popd    
)

if "%PLATFORM%" == "PS4" (
::    call %~dp0config-vs-legacy.bat 14.0
    call %~dp0config-vs-x64.bat
    pushd "%TRAKTOR_HOME%\build\ps4"
    msbuild "Extern Ps4.sln" /nologo /maxcpucount /p:BuildInParallel=true /p:Configuration=%CONFIG% /p:Platform="ORBIS"
    msbuild "Traktor Ps4.sln" /nologo /maxcpucount /p:BuildInParallel=true /p:Configuration=%CONFIG% /p:Platform="ORBIS"
    popd
)