@echo off

call %~dp0../config.bat

pushd %TRAKTOR_HOME%

mkdir bin\latest\win64\debugshared
mkdir bin\latest\win64\releaseshared
mkdir bin\latest\win64\releasestatic

:: DebugShared
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\win64\debugshared\*.dll" bin\latest\win64\debugshared
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\win64\debugshared\*.exe" bin\latest\win64\debugshared
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\win64\debugshared\*.pdb" bin\latest\win64\debugshared
xcopy /C /I /R /Y "%STEAMWORKS_SDK%\redistributable_bin\win64\steam_api64.dll" bin\latest\win64\debugshared
xcopy /C /I /R /Y "%FBX_SDK%\lib\vs2008\x64\*.dll" bin\latest\win64\debugshared
xcopy /C /I /R /Y "%TRAKTOR_HOME%3rdp\DirectX Redist (8.0)\x64\*.dll" bin\latest\win64\debugshared
xcopy /C /I /R /Y "%SCE_PS3_ROOT%\host-win32\Cg\bin\libcgc_x64.dll" bin\latest\win32\debugshared

:: ReleaseShared
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\win64\releaseshared\*.dll" bin\latest\win64\releaseshared
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\win64\releaseshared\*.exe" bin\latest\win64\releaseshared
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\win64\releaseshared\*.pdb" bin\latest\win64\releaseshared
xcopy /C /I /R /Y "%STEAMWORKS_SDK%\redistributable_bin\win64\steam_api64.dll" bin\latest\win64\releaseshared
xcopy /C /I /R /Y "%FBX_SDK%\lib\vs2008\x64\*.dll" bin\latest\win64\releaseshared
xcopy /C /I /R /Y "%TRAKTOR_HOME%3rdp\DirectX Redist (8.0)\x64\*.dll" bin\latest\win64\releaseshared
xcopy /C /I /R /Y "%SCE_PS3_ROOT%\host-win32\Cg\bin\libcgc_x64.dll" bin\latest\win64\releaseshared

:: ReleaseStatic
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\win64\releasestatic\*.lib" bin\latest\win64\releasestatic
xcopy /C /I /R /Y "%STEAMWORKS_SDK%\redistributable_bin\win64\steam_api64.lib" bin\latest\win64\releasestatic
xcopy /C /I /R /Y "%STEAMWORKS_SDK%\redistributable_bin\win64\steam_api64.dll" bin\latest\win64\releasestatic
xcopy /C /I /R /Y "%FBX_SDK%\lib\vs2008\x64\*.dll" bin\latest\win64\releasestatic
xcopy /C /I /R /Y "%TRAKTOR_HOME%3rdp\DirectX Redist (8.0)\x64\*.dll" bin\latest\win64\releasestatic
xcopy /C /I /R /Y "%SCE_PS3_ROOT%\host-win32\Cg\bin\libcgc_x64.dll" bin\latest\win64\releasestatic

popd
