@echo off

call "%~dp0..\config.bat"

pushd %TRAKTOR_HOME%

rd /Q /S "resources\documentation\latest" 2> nul || rem

if %ERRORLEVEL% equ 1 (
	popd
	cmd /C exit 0
)
if %ERRORLEVEL% equ 2 (
	popd
	cmd /C exit 0
)
if %ERRORLEVEL% equ 3 (
	popd
	cmd /C exit 0
)
if %ERRORLEVEL% neq 0 ( 
	echo Unable to remove old documentation output.
	echo %ERRORLEVEL%
	popd
	exit /B 1
)

popd

echo Clean successful
