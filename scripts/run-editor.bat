@echo off

:: Source environment configuration.
call %~dp0config.bat
call %~dp0config-android.bat

:: Update all binaries.
set TC_HOST=http://pn-tambor-ws.pistol.one:8111

%TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.Run.App $(TRAKTOR_HOME)/scripts/runtime/targets/sync-teamcity-latest.run %TC_HOST% $(TRAKTOR_HOME)/bin/latest/android Traktor_Android
%TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.Run.App $(TRAKTOR_HOME)/scripts/runtime/targets/sync-teamcity-latest.run %TC_HOST% $(TRAKTOR_HOME)/bin/latest/ios Traktor_IOS
%TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.Run.App $(TRAKTOR_HOME)/scripts/runtime/targets/sync-teamcity-latest.run %TC_HOST% $(TRAKTOR_HOME)/bin/latest/linux Traktor_Linux
%TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.Run.App $(TRAKTOR_HOME)/scripts/runtime/targets/sync-teamcity-latest.run %TC_HOST% $(TRAKTOR_HOME)/bin/latest/osx Traktor_OSX
%TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.Run.App $(TRAKTOR_HOME)/scripts/runtime/targets/sync-teamcity-latest.run %TC_HOST% $(TRAKTOR_HOME)/bin/latest/rpi Traktor_RaspberryPi
:: %TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.Run.App $(TRAKTOR_HOME)/scripts/runtime/targets/sync-teamcity-latest.run %TC_HOST% $(TRAKTOR_HOME)/bin/latest/win64 Traktor_Win64

:: Launch editor.
:: start %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Editor.App.exe
