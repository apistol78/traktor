@echo off

call %~dp0config.bat

call build-system-assets-sw.bat

%TRAKTOR_HOME%bin\CeDeploy "\Program Files\SceneViewer" $(TRAKTOR_HOME)/samples/SceneViewer/build/mobile6/ReleaseStatic/SceneViewer.exe
%TRAKTOR_HOME%bin\CeLaunch "\Program Files\SceneViewer\SceneViewer" -x=240 -y=320 -m=192.168.143.23:33666/Output-Sw %1
