@echo off

call %~dp0../config.bat

pushd %TRAKTOR_HOME%

mkdir bin\latest\win32\debugshared
mkdir bin\latest\win32\releaseshared

:: DebugShared
xcopy /C /I /R /Y "%TRAKTOR_HOME%\3rdp\POWERVR SDK\OGLES2_WINDOWS_PCEMULATION_2.07.27.0484\Utilities\PVRTC\dll\pvrtc.dll" bin\latest\win32\debugshared
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\win32\debugshared\*.dll" bin\latest\win32\debugshared
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\win32\debugshared\*.exe" bin\latest\win32\debugshared
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\win32\debugshared\*.pdb" bin\latest\win32\debugshared
xcopy /C /I /R /Y "%STEAMWORKS_SDK%\redistributable_bin\steam_api.dll" bin\latest\win32\debugshared
xcopy /C /I /R /Y "%PHYSX_SDK%\bin\win32\*.dll" bin\latest\win32\debugshared
xcopy /C /I /R /Y "%FBX_SDK%\lib\vs2008\x86\*.dll" bin\latest\win32\debugshared
xcopy /C /I /R /Y "%TRAKTOR_HOME%3rdp\DirectX Redist (8.0)\x86\*.dll" bin\latest\win32\debugshared

:: ReleaseShared
xcopy /C /I /R /Y "%TRAKTOR_HOME%\3rdp\POWERVR SDK\OGLES2_WINDOWS_PCEMULATION_2.07.27.0484\Utilities\PVRTC\dll\pvrtc.dll" bin\latest\win32\releaseshared
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\win32\releaseshared\*.dll" bin\latest\win32\releaseshared
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\win32\releaseshared\*.exe" bin\latest\win32\releaseshared
xcopy /C /I /R /Y "%TRAKTOR_HOME%\build\win32\releaseshared\*.pdb" bin\latest\win32\releaseshared
xcopy /C /I /R /Y "%STEAMWORKS_SDK%\redistributable_bin\steam_api.dll" bin\latest\win32\releaseshared
xcopy /C /I /R /Y "%PHYSX_SDK%\bin\win32\*.dll" bin\latest\win32\releaseshared
xcopy /C /I /R /Y "%FBX_SDK%\lib\vs2008\x86\*.dll" bin\latest\win32\releaseshared
xcopy /C /I /R /Y "%TRAKTOR_HOME%3rdp\DirectX Redist (8.0)\x86\*.dll" bin\latest\win32\releaseshared

popd
