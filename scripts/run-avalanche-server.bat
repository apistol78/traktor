@echo off

call %~dp0config.bat

%TRAKTOR_HOME%/bin/latest/win64/releaseshared/Traktor.Avalanche.Server.App -d=/temp/avalanche
