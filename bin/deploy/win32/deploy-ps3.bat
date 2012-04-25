@echo off

set PACKAGE=%DEPLOY_PROJECT_NAME: =%

if "%1"=="build" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2 %3 %4 %5

) else if "%1"=="deploy" (

	rem Deploy PS3 linkage script.
	pushd %TRAKTOR_HOME%\bin\win32
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% link-ps3.bat >> %DEPLOY_PROJECT_ROOT%\deploy.log
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% launch-ps3.bat >> %DEPLOY_PROJECT_ROOT%\deploy.log
	popd

	rem Deploy PS3 binaries.
	pushd %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\ps3\releasestatic
	%TRAKTOR_HOME%\bin\RemoteDeploy -recursive -target-base="ppu" %DEPLOY_TARGET_HOST% *.lib >> %DEPLOY_PROJECT_ROOT%\deploy.log
	popd
	pushd %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\ps3-spu\releasestatic
	%TRAKTOR_HOME%\bin\RemoteDeploy -recursive -target-base="spu" %DEPLOY_TARGET_HOST% *.lib >> %DEPLOY_PROJECT_ROOT%\deploy.log
	popd

	rem Link PS3 binary.
	%TRAKTOR_HOME%\bin\RemoteLaunch -w %DEPLOY_TARGET_HOST% link-ps3.bat "%PACKAGE% %DEPLOY_EXECUTABLE% %DEPLOY_MODULES%" >> %DEPLOY_PROJECT_ROOT%\deploy.log

) else if "%1"=="launch" (

	rem Deploy configuration and then launch PS3 binary.
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% Application.config > %DEPLOY_PROJECT_ROOT%\launch.log
	%TRAKTOR_HOME%\bin\RemoteLaunch -w %DEPLOY_TARGET_HOST% launch-ps3.bat "%DEPLOY_EXECUTABLE%.self Application.config" >> %DEPLOY_PROJECT_ROOT%\launch.log

) else if "%1"=="migrate" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Database.Migrate.App %DEPLOY_SOURCE_CS% %DEPLOY_TARGET_CS% %DEPLOY_MODULES% > %DEPLOY_PROJECT_ROOT%\migrate.log
	
)

