@echo off

call %~dp0../config.bat

pushd %TRAKTOR_HOME%

mkdir bin\latest\android\releasestatic

rem ReleaseStatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\android\releasestatic\*.a bin\latest\android\releasestatic

popd
