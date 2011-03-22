@echo off

if "%1"=="build" (

	%DEPLOY_PROJECTROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2

) else if "%1"=="deploy" (

	pushd %DEPLOY_PROJECTROOT:/=\%\bin\latest\win32\releaseshared
	%TRAKTOR_HOME%\bin\RemoteDeploy localhost *.dll *.exe > %DEPLOY_PROJECTROOT%\deploy.log
	popd

	rem if not exist bin ( mkdir bin )
	rem xcopy /Q /R /Y %DEPLOY_PROJECTROOT:/=\%\bin\latest\win32\releaseshared\*.dll bin\
	rem xcopy /Q /R /Y %DEPLOY_PROJECTROOT:/=\%\bin\latest\win32\releaseshared\*.exe bin\
	rem set ERRORLEVEL=0
	
) else if "%1"=="launch" (

	%TRAKTOR_HOME%\bin\RemoteDeploy localhost Application.config
	%TRAKTOR_HOME%\bin\RemoteLaunch localhost "%DEPLOY_EXECUTABLE%" "\-s Application.config"

	rem bin\%DEPLOY_EXECUTABLE% -s Application.config
	
)
