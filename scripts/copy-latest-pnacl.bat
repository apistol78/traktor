@echo off

call %~dp0../config.bat

pushd %TRAKTOR_HOME%

mkdir bin\latest\pnacl\releasestatic

rem ReleaseStatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\pnacl\releasestatic\*.a bin\latest\pnacl\releasestatic

popd
