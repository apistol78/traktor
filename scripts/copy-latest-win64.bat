@echo off

call %~dp0../config.bat

pushd %TRAKTOR_HOME%

mkdir bin\latest\win64\debugshared
mkdir bin\latest\win64\releaseshared

rem DebugShared
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\win64\debugshared\*.dll bin\latest\win64\debugshared
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\win64\debugshared\*.exe bin\latest\win64\debugshared
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\win64\debugshared\*.pdb bin\latest\win64\debugshared
xcopy /C /I /R /Y %STEAMWORKS_SDK%\redistributable_bin\win64\steam_api64.dll bin\latest\win64\debugshared

rem ReleaseShared
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\win64\releaseshared\*.dll bin\latest\win64\releaseshared
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\win64\releaseshared\*.exe bin\latest\win64\releaseshared
xcopy /C /I /R /Y %TRAKTOR_HOME%\build\win64\releaseshared\*.pdb bin\latest\win64\releaseshared
xcopy /C /I /R /Y %STEAMWORKS_SDK%\redistributable_bin\win64\steam_api64.dll bin\latest\win64\releaseshared

popd
