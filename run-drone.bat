@echo off

call %~dp0config.bat

start %TRAKTOR_HOME%\bin\Drone.App.exe $(TRAKTOR_HOME)/Drone.App.config
