@echo off

call %~dp0config.bat

:: Launch drone.
start %SHADOWLAUNCH% %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Drone.App.exe $(TRAKTOR_HOME)/resources/runtime/configurations/Traktor.Drone.config
