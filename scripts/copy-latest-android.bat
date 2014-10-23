@echo off

call %~dp0../config.bat

pushd %TRAKTOR_HOME%

mkdir bin\latest\android\debugstatic
mkdir bin\latest\android\releasestatic

:: Copy binaries
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\android\debugstatic\*.a" bin\latest\android\debugstatic
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\android\releasestatic\*.a" bin\latest\android\releasestatic

popd
