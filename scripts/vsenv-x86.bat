@echo off

:: Locate VS environment script.
set VSWHERE="%~dp0..\3rdp\Visual Studio\vswhere"
for /f "usebackq delims=" %%i in (`%VSWHERE% -prerelease -latest -property installationPath`) do (
	if exist "%%i\Common7\Tools\vsdevcmd.bat" (
		set VSDEVCMD="%%i\Common7\Tools\vsdevcmd.bat"
	)
)

:: Setup VC environment variables.
pushd
set VSCMD_START_DIR=%CD%
call %VSDEVCMD%
popd