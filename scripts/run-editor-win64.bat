@echo off

:: Source environment configuration.
call %~dp0config.bat
call %~dp0config-android.bat
call %~dp0config-emscripten.bat

:: Launch editor.
start %TRAKTOR_HOME%\bin\latest\win64\releaseshared\Traktor.Editor.App.exe
