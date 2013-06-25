@echo off

if "%1"=="build" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2 %3 %4 %5

) else if "%1"=="deploy" (

	call %TRAKTOR_HOME%\bin\win32\link-android.bat %DEPLOY_EXECUTABLE% %DEPLOY_MODULES% > "%DEPLOY_OUTPUT_PATH%\Deploy.log"

) else if "%1"=="launch" (

	echo "Not implemented"

) else if "%1"=="migrate" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Database.Migrate.App -s=Migrate -l=Migrate.log

)
