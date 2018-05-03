@echo off
set VSWHERE="%~dp0..\3rdp\Visual Studio\vswhere"

:: Legacy environment script.
for /f "usebackq delims=" %%i in (`%VSWHERE% -prerelease -legacy -version %1 -property installationPath`) do (
	if exist "%%i\VC\vcvarsall.bat" (
		set VSDEVCMD=%%i\VC\vcvarsall.bat
	)
)

if "%VSDEVCMD%" == "" ( goto no_vs )
call "%VSDEVCMD%"
goto eof

:no_vs

echo No legacy Visual Studio found.
exit /b 1

:eof
