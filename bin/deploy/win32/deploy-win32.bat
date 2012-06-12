@echo off

if "%1"=="build" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2 %3 %4 %5

) else if "%1"=="deploy" (

	pushd %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\win32\releaseshared
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% *.dll *.exe *.txt > Deploy.log
	popd
	
) else if "%1"=="launch" (

	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% Application.config
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch %DEPLOY_TARGET_HOST% "%DEPLOY_EXECUTABLE%" "\-s Application.config"

) else if "%1"=="migrate" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Database.Migrate.App -s=Migrate -l=Migrate.log

)
