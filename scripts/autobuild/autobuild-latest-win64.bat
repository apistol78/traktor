@echo off

set CONFIG=%*
if "%CONFIG%" neq "" ( set CONFIG="/p:Configuration=%CONFIG%" )

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\config-vs-x64.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\win64"
msbuild -m "Traktor Win64.sln" %CONFIG%
if %ERRORLEVEL% neq 1 ( exit 1 )
popd
