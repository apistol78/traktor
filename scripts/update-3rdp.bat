@echo off

call %~dp0config.bat

:: Ensure GIT is installed first.
where /q git
if errorlevel 1 (
	echo GIT client is missing. Ensure it is installed and placed in your PATH.
	echo Tip! "winget install --id Git.Git -e --source winget"
	exit /b
)

pushd %TRAKTOR_HOME%
%TRAKTOR_HOME%/bin/win64/releasestatic/Traktor.Run.App ./scripts/misc/update-3rdp.run
popd
