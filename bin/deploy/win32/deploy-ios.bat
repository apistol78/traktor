@echo off

if "%1"=="build" (

	%DEPLOY_PROJECTROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2

) else if "%1"=="deploy" (

	echo "Not available"
	
) else if "%1"=="launch" (

	echo "Not available"

) else if "%1"=="migrate" (

	%DEPLOY_PROJECTROOT%\bin\latest\win32\releaseshared\Traktor.Database.Migrate.App %DEPLOY_SOURCE_CS% %DEPLOY_TARGET_CS% %DEPLOY_MODULES% > %DEPLOY_PROJECTROOT%\migrate.log
	
)

