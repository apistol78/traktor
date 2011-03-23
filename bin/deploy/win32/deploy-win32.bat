@echo off

if "%1"=="build" (

	%DEPLOY_PROJECTROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2

) else if "%1"=="deploy" (

	pushd %DEPLOY_PROJECTROOT:/=\%\bin\latest\win32\releaseshared
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST_WIN32% *.dll *.exe > %DEPLOY_PROJECTROOT%\deploy.log
	popd
	
) else if "%1"=="launch" (

	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST_WIN32% Application.config
	%TRAKTOR_HOME%\bin\RemoteLaunch %DEPLOY_TARGET_HOST_WIN32% "%DEPLOY_EXECUTABLE%" "\-s Application.config"
	
)
