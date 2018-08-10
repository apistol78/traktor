@echo off

call %~dp0config.bat

set SH=%TRAKTOR_HOME%\bin\win64\ShadowLaunch

%SH% TraktorDrone %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Drone.App.exe $(TRAKTOR_HOME)/resources/runtime/configurations/Traktor.Drone.config
