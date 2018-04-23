@echo off

call "%~dp0..\config.bat"

pushd %TRAKTOR_HOME%

rd /Q /S "build\win64" 2> nul || rem
if %ERRORLEVEL% equ 2 ( cmd /C exit 0 )
if %ERRORLEVEL% equ 3 ( cmd /C exit 0 )
if %ERRORLEVEL% neq 0 ( exit /B 1 )

rd /Q /S "bin\latest\win64" 2> nul || rem
if %ERRORLEVEL% equ 2 ( cmd /C exit 0 )
if %ERRORLEVEL% equ 3 ( cmd /C exit 0 )
if %ERRORLEVEL% neq 0 ( exit /B 1 )

popd

echo Clean successful
