::@echo off

:: Setup our build environment.
call "%~dp0..\config.bat"

mkdir "%TRAKTOR_HOME%/build/archive/traktor"
pushd "%TRAKTOR_HOME%/build/archive/traktor"

xcopy "%TRAKTOR_HOME%\bin\latest\win64\releaseshared\" "bin\latest\win64\releaseshared\" /S /E /Y /R
xcopy "%TRAKTOR_HOME%\data\Assets\" "data\Assets\" /S /E /Y /R
xcopy "%TRAKTOR_HOME%\data\Source\" "data\Source\" /S /E /Y /R
xcopy "%TRAKTOR_HOME%\resources\runtime\" "resources\runtime\" /S /E /Y /R
xcopy "%TRAKTOR_HOME%\scripts\runtime\" "scripts\runtime\" /S /E /Y /R
xcopy "%TRAKTOR_HOME%\LICENSE.txt" "." /Y /R
xcopy "%TRAKTOR_HOME%\README.md" "." /Y /R
powershell Compress-Archive . ..\Traktor.zip

popd