@echo off

call %~dp0../config.bat

pushd %TRAKTOR_HOME%

mkdir bin\latest\mobile6\debugshared
mkdir bin\latest\mobile6\releaseshared

rem DebugShared
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\mobile6\debugshared\*.dll bin\latest\mobile6\debugshared
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\mobile6\debugshared\*.exe bin\latest\mobile6\debugshared
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\mobile6\debugshared\*.pdb bin\latest\mobile6\debugshared

rem ReleaseShared
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\mobile6\releaseshared\*.dll bin\latest\mobile6\releaseshared
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\mobile6\releaseshared\*.exe bin\latest\mobile6\releaseshared
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\mobile6\releaseshared\*.pdb bin\latest\mobile6\releaseshared

popd
