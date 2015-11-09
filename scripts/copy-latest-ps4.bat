@echo off

call %~dp0../config.bat

:: debugstatic
pushd %TRAKTOR_HOME%
mkdir bin\latest\ps4\debugstatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\ps4\debugstatic\*.a bin\latest\ps4\debugstatic
popd

:: releasestatic
pushd %TRAKTOR_HOME%
mkdir bin\latest\ps4\releasestatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\ps4\releasestatic\*.a bin\latest\ps4\releasestatic
popd
