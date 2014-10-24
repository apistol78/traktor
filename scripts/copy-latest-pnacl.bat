@echo off

call %~dp0../config.bat

:: debugstatic
pushd %TRAKTOR_HOME%
mkdir bin\latest\pnacl\debugstatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\pnacl\debugstatic\*.a bin\latest\pnacl\debugstatic
popd

:: releasestatic
pushd %TRAKTOR_HOME%
mkdir bin\latest\pnacl\releasestatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\pnacl\releasestatic\*.a bin\latest\pnacl\releasestatic
popd
