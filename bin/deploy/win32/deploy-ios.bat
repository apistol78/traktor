@echo off

if "%1"=="build" (

	%DEPLOY_PROJECTROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2

) else if "%1"=="deploy" (

	rem Deploy iPhone simulator launcher.
	pushd %TRAKTOR_HOME%\bin\osx
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% waxsim > %DEPLOY_PROJECTROOT%\deploy.log
	popd

	rem Deploy iPhone binaries.
	pushd %DEPLOY_PROJECTROOT:/=\%\bin\latest\iphone-simulator\releasestatic
	%TRAKTOR_HOME%\bin\RemoteDeploy -recursive %DEPLOY_TARGET_HOST% *.* >> %DEPLOY_PROJECTROOT%\deploy.log
	popd
	
) else if "%1"=="launch" (

	rem Launch application with simulator launcher on target.
	%TRAKTOR_HOME%\bin\RemoteDeploy -target-base="%DEPLOY_EXECUTABLE%.app" %DEPLOY_TARGET_HOST% Application.config > %DEPLOY_PROJECTROOT%\launch.log
	%TRAKTOR_HOME%\bin\RemoteLaunch %DEPLOY_TARGET_HOST% waxsim "%DEPLOY_EXECUTABLE%.app" >> %DEPLOY_PROJECTROOT%\launch.log

) else if "%1"=="migrate" (

	%DEPLOY_PROJECTROOT%\bin\latest\win32\releaseshared\Traktor.Database.Migrate.App %DEPLOY_SOURCE_CS% %DEPLOY_TARGET_CS% %DEPLOY_MODULES% > %DEPLOY_PROJECTROOT%\migrate.log
	
)
