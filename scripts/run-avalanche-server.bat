@echo off

call %~dp0config.bat

%TRAKTOR_HOME%/bin/win64/releasestatic/Traktor.Avalanche.Server.App -d=/temp/avalanche
