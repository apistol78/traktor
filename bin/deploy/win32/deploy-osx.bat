@echo off

if "%1"=="build" (

	%DEPLOY_PROJECTROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2

) else if "%1"=="deploy" (

	pushd %DEPLOY_PROJECTROOT:/=\%\bin\latest\osx\releaseshared
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST_OSX% * > %DEPLOY_PROJECTROOT%\deploy.log
	popd

) else if "%1"=="launch" (

	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST_OSX% Application.config > %DEPLOY_PROJECTROOT%\deploy.log
	%TRAKTOR_HOME%\bin\RemoteLaunch %DEPLOY_TARGET_HOST_OSX% %DEPLOY_EXECUTABLE% "\-s Application.config" >> %DEPLOY_PROJECTROOT%\deploy.log

)
