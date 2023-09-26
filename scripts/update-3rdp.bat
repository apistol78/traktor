@echo off

call %~dp0config.bat

:: Ensure GIT is installed first.
winget install --id Git.Git -e --source winget

pushd %TRAKTOR_HOME%
%TRAKTOR_HOME%/bin/win64/releasestatic/Traktor.Run.App ./scripts/misc/update-3rdp.run
popd
