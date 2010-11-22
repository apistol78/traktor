@echo off

set REMOTE_HOST=192.168.143.12
set REMOTE_USER=pistol
set REMOTE_PWD=dolphin
set REMOTE_PATH=/Users/pistol/AmalgamRemoteDeploy
set REMOTE_TARGET=%REMOTE_USER%@%REMOTE_HOST%:%REMOTE_PATH%

set PSCP=%AMALGAM_HOME%/3rdp/putty/pscp -pw %REMOTE_PWD% -r -batch
set PLINK=%AMALGAM_HOME%/3rdp/putty/plink -pw %REMOTE_PWD% %REMOTE_USER%@%REMOTE_HOST%

if "%1"=="build" (

	%DEPLOY_PROJECTROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Traktor.Pipeline -l=Pipeline.log %2

) else if "%1"=="deploy" (

	%PLINK% "mkdir %REMOTE_PATH%"
	%PLINK% "mkdir %REMOTE_PATH%/Amalgam.app"
	%PLINK% "mkdir %REMOTE_PATH%/Amalgam.app/Contents"
	%PLINK% "mkdir %REMOTE_PATH%/Amalgam.app/Contents/MacOS"

	%PSCP% Application.config %REMOTE_TARGET%/Application.config
	%PSCP% %TRAKTOR_HOME%/build/macosx/releaseshared/*.dylib %REMOTE_TARGET%/Amalgam.app/Contents/MacOS
	%PSCP% %AMALGAM_HOME%/build/macosx/releaseshared/*.dylib %REMOTE_TARGET%/Amalgam.app/Contents/MacOS
	%PSCP% %AMALGAM_HOME%/build/macosx/releaseshared/Amalgam.Viewer %REMOTE_TARGET%/Amalgam.app/Contents/MacOS
	%PSCP% %AMALGAM_HOME%/targets/macosx-ogl/PkgInfo %REMOTE_TARGET%/Amalgam.app/Contents
	%PSCP% %AMALGAM_HOME%/targets/macosx-ogl/Info.plist %REMOTE_TARGET%/Amalgam.app/Contents

	%PLINK% "chmod +x %REMOTE_PATH%/Amalgam.app/Contents/MacOS/Amalgam.Viewer"

) else if "%1"=="launch" (

	%PLINK% "%REMOTE_PATH%/Amalgam.app/Contents/MacOS/Amalgam.Viewer /Users/pistol/AmalgamRemoteDeploy/Application.config"

) else (
	echo "Usage: [deploy] command (args...)"
)
