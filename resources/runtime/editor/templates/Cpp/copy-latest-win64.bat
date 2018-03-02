@echo off

call %~dp0config.bat

pushd %GAME_HOME%

:: Recreate destinations
mkdir bin\latest\win64\debugshared
mkdir bin\latest\win64\releaseshared
mkdir bin\latest\win64\releasestatic

:: DebugShared
xcopy /D /C /I /R /Y "%GAME_HOME%\build\win64\debugshared\*.dll" bin\latest\win64\debugshared
xcopy /D /C /I /R /Y "%GAME_HOME%\build\win64\debugshared\*.exe" bin\latest\win64\debugshared
xcopy /D /C /I /R /Y "%GAME_HOME%\build\win64\debugshared\*.pdb" bin\latest\win64\debugshared
xcopy /D /C /I /R /Y "%TRAKTOR_HOME%\build\win64\debugshared\*.dll" bin\latest\win64\debugshared
xcopy /D /C /I /R /Y "%TRAKTOR_HOME%\build\win64\debugshared\*.exe" bin\latest\win64\debugshared
xcopy /D /C /I /R /Y "%TRAKTOR_HOME%\build\win64\debugshared\*.pdb" bin\latest\win64\debugshared
xcopy /D /C /I /R /Y "%STEAMWORKS_SDK%\redistributable_bin\win64\steam_api64.dll" bin\latest\win64\debugshared
xcopy /D /C /I /R /Y "%PHYSX_SDK%\bin\win64\*.dll" bin\latest\win64\debugshared
xcopy /D /C /I /R /Y "%FBX_SDK%\lib\vs2008\x64\*.dll" bin\latest\win64\debugshared
xcopy /D /C /I /R /Y "%TRAKTOR_HOME%3rdp\DirectX Redist (8.0)\x64\*.dll" bin\latest\win64\debugshared
xcopy /D /C /I /R /Y "%SCE_PS3_ROOT%\host-win32\Cg\bin\libcgc_x64.dll" bin\latest\win64\debugshared
xcopy /D /C /I /R /Y "%GAME_HOME%\scripts\*.txt" bin\latest\win64\debugshared
xcopy /D /C /I /R /Y "%SIMPLYGON_SDK%\SimplygonSDKRuntimeReleasex64.dll" bin\latest\win64\debugshared

:: ReleaseShared
xcopy /D /C /I /R /Y "%GAME_HOME%\build\win64\releaseshared\*.dll" bin\latest\win64\releaseshared
xcopy /D /C /I /R /Y "%GAME_HOME%\build\win64\releaseshared\*.exe" bin\latest\win64\releaseshared
xcopy /D /C /I /R /Y "%GAME_HOME%\build\win64\releaseshared\*.pdb" bin\latest\win64\releaseshared
xcopy /D /C /I /R /Y "%TRAKTOR_HOME%\build\win64\releaseshared\*.dll" bin\latest\win64\releaseshared
xcopy /D /C /I /R /Y "%TRAKTOR_HOME%\build\win64\releaseshared\*.exe" bin\latest\win64\releaseshared
xcopy /D /C /I /R /Y "%TRAKTOR_HOME%\build\win64\releaseshared\*.pdb" bin\latest\win64\releaseshared
xcopy /D /C /I /R /Y "%STEAMWORKS_SDK%\redistributable_bin\win64\steam_api64.dll" bin\latest\win64\releaseshared
xcopy /D /C /I /R /Y "%PHYSX_SDK%\bin\win64\*.dll" bin\latest\win64\releaseshared
xcopy /D /C /I /R /Y "%FBX_SDK%\lib\vs2008\x64\*.dll" bin\latest\win64\releaseshared
xcopy /D /C /I /R /Y "%TRAKTOR_HOME%3rdp\DirectX Redist (8.0)\x64\*.dll" bin\latest\win64\releaseshared
xcopy /D /C /I /R /Y "%SCE_PS3_ROOT%\host-win32\Cg\bin\libcgc_x64.dll" bin\latest\win64\releaseshared
xcopy /D /C /I /R /Y "%GAME_HOME%\scripts\*.txt" bin\latest\win64\releaseshared
xcopy /D /C /I /R /Y "%SIMPLYGON_SDK%\SimplygonSDKRuntimeReleasex64.dll" bin\latest\win64\releaseshared

:: ReleaseStatic
xcopy /D /C /I /R /Y "%GAME_HOME%\build\win64\releasestatic\*.lib" bin\latest\win64\releasestatic
xcopy /D /C /I /R /Y "%TRAKTOR_HOME%\build\win64\releasestatic\*.lib" bin\latest\win64\releasestatic
xcopy /D /C /I /R /Y "%STEAMWORKS_SDK%\redistributable_bin\win64\steam_api64.lib" bin\latest\win64\releasestatic
xcopy /D /C /I /R /Y "%STEAMWORKS_SDK%\redistributable_bin\win64\steam_api64.dll" bin\latest\win64\releasestatic
xcopy /D /C /I /R /Y "%PHYSX_SDK%\bin\win64\*.dll" bin\latest\win64\releasestatic
xcopy /D /C /I /R /Y "%FBX_SDK%\lib\vs2008\x64\*.dll" bin\latest\win64\releasestatic
xcopy /D /C /I /R /Y "%TRAKTOR_HOME%3rdp\DirectX Redist (8.0)\x64\*.dll" bin\latest\win64\releasestatic
xcopy /D /C /I /R /Y "%SCE_PS3_ROOT%\host-win32\Cg\bin\libcgc_x64.dll" bin\latest\win64\releasestatic
xcopy /D /C /I /R /Y "%GAME_HOME%\scripts\*.txt" bin\latest\win64\releasestatic
xcopy /D /C /I /R /Y "%SIMPLYGON_SDK%\SimplygonSDKRuntimeReleasex64.dll" bin\latest\win64\releasestatic

popd
