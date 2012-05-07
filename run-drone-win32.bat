@echo off

call %~dp0config.bat

start %TRAKTOR_HOME%\bin\win32\Drone.App.exe $(TRAKTOR_HOME)/Drone.App.config
