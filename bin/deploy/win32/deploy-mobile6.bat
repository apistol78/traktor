@echo off

if "%1"=="build" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2 %3 %4 %5

) else if "%1"=="deploy" (

	%TRAKTOR_HOME%bin\win32\CeDeploy "\Program Files\%DEPLOY_PROJECT_NAME%" %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\mobile6\releaseshared\*.dll > %DEPLOY_PROJECT_ROOT%\deploy.log
	%TRAKTOR_HOME%bin\win32\CeDeploy "\Program Files\%DEPLOY_PROJECT_NAME%" %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\mobile6\releaseshared\*.exe >> %DEPLOY_PROJECT_ROOT%\deploy.log
	%TRAKTOR_HOME%bin\win32\CeDeploy "\Program Files\%DEPLOY_PROJECT_NAME%" Application.config >> %DEPLOY_PROJECT_ROOT%\deploy.log

) else if "%1"=="launch" (

	%TRAKTOR_HOME%bin\win32\CeLaunch "\Program Files\%DEPLOY_PROJECT_NAME%\%DEPLOY_EXECUTABLE%" Application.config > %DEPLOY_PROJECT_ROOT%\launch.log
	
) else if "%1"=="migrate" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Database.Migrate.App %DEPLOY_SOURCE_CS% %DEPLOY_TARGET_CS% %DEPLOY_MODULES% > %DEPLOY_PROJECT_ROOT%\migrate.log
	
)

