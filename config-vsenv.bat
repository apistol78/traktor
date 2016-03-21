@echo off
call %~dp0config.bat

set FILE="%1"
set /p VERSION=< %FILE%

:: -----------------------------------------------------------------------------

if /i not "%VERSION%" == "vs2012_x64" ( goto :skip_vs2012_x64 )
echo Set VS2012 (x64) environment

@set FBX_SDK_LIBPATH=%FBX_SDK%\lib\vs2012\x64\release
@set P4_SDK=%TRAKTOR_HOME%3rdp\p4api-2016.1.1350954.BETA-vs2012_dyn_x64

:skip_vs2012_x64

:: -----------------------------------------------------------------------------

if /i not "%VERSION%" == "vs2012_x86" ( goto :skip_vs2012_x86 )
echo Set VS2012 (x86) environment

@set FBX_SDK_LIBPATH=%FBX_SDK%\lib\vs2012\x86\release
@set P4_SDK=%TRAKTOR_HOME%3rdp\p4api-2016.1.1350954.BETA-vs2012_dyn_x86

:skip_vs2012_x86

:: -----------------------------------------------------------------------------

if /i not "%VERSION%" == "vs2015_x64" ( goto :skip_vs2015_x64 )
echo Set VS2015 (x64) environment

@set FBX_SDK_LIBPATH=%FBX_SDK%\lib\vs2015\x64\release
@set P4_SDK=%TRAKTOR_HOME%3rdp\p4api-2016.1.1350954.BETA-vs2015_dyn_x64

:skip_vs2015_x64

:: -----------------------------------------------------------------------------

if /i not "%VERSION%" == "vs2015_x86" ( goto :skip_vs2015_x86 )
echo Set VS2015 (x86) environment

@set FBX_SDK_LIBPATH=%FBX_SDK%\lib\vs2015\x86\release
@set P4_SDK=%TRAKTOR_HOME%3rdp\p4api-2016.1.1350954.BETA-vs2015_dyn_x86

:skip_vs2015_x86
