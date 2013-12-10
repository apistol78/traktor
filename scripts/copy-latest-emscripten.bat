@echo off

call %~dp0../config.bat

pushd %TRAKTOR_HOME%

mkdir bin\latest\emscripten\releasestatic

rem ReleaseStatic
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\emscripten\releasestatic\*.bc bin\latest\emscripten\releasestatic

popd
