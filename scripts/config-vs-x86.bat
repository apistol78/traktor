@echo off
set VSWHERE="%~dp0..\3rdp\Visual Studio\vswhere"

:: Check if already setup.
if "%HAVE_CONFIG_VS%" == "x86" (
	goto eof
)

:: Non-legacy 2017+ environment script.
for /f "usebackq delims=" %%i in (`%VSWHERE% -prerelease -latest -property installationPath`) do (
	if exist "%%i\Common7\Tools\vsdevcmd.bat" (
		set VSDEVCMD_2017=%%i\Common7\Tools\vsdevcmd.bat
	)
)

:: Legacy pre-2017 environment script.
for /f "usebackq delims=" %%i in (`%VSWHERE% -prerelease -legacy -latest -property installationPath`) do (
	if exist "%%i\VC\vcvarsall.bat" (
		set VSDEVCMD_LEGACY=%%i\VC\vcvarsall.bat
	)
)

if "%VSDEVCMD_2017%" == "" ( goto no_vs2017 )

pushd
set VSCMD_START_DIR=%CD%
call "%VSDEVCMD_2017%" -no_logo
popd
goto eof

:no_vs2017

if "%VSDEVCMD_LEGACY%" == "" ( goto no_vs2015 )

call "%VSDEVCMD_LEGACY%"
set PATH=%PATH%;"%WindowsSdkDir%bin\%WindowsSDKVersion%x86"
goto eof

:no_vs2015

echo No Visual Studio found.
exit /b 1

:eof
set HAVE_CONFIG_VS=x86
