@echo off

call %~dp0config.bat

start %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Drone.App.exe $(TRAKTOR_HOME)/Drone.App.config
