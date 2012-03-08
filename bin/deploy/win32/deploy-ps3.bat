@echo off

if "%1"=="build" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2 %3 %4 %5

) else if "%1"=="deploy" (

	if not exist bin ( mkdir bin )
	xcopy /Q /R /Y %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\ps3\releasestatic\*.self bin
	
) else if "%1"=="launch" (

	ps3run -k -r -f . bin/%DEPLOY_EXECUTABLE%.self Application.config
	
) else if "%1"=="migrate" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Database.Migrate.App %DEPLOY_SOURCE_CS% %DEPLOY_TARGET_CS% %DEPLOY_MODULES% > %DEPLOY_PROJECT_ROOT%\migrate.log
	
)

