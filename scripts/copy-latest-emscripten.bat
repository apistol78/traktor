@echo off

call %~dp0../config.bat

pushd %TRAKTOR_HOME%

mkdir bin\latest\emscripten\debugstatic
mkdir bin\latest\emscripten\releasestatic

:: Copy binaries
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\emscripten\debugstatic\*.a" bin\latest\emscripten\debugstatic
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\emscripten\releasestatic\*.a" bin\latest\emscripten\releasestatic

popd
