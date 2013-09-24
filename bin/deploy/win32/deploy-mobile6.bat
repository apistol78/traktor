@echo off

if "%1"=="build" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2 %3 %4 %5

	if not exist "%DEPLOY_OUTPUT_PATH%\reports" mkdir "%DEPLOY_OUTPUT_PATH%\reports"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Textures.html > "%DEPLOY_OUTPUT_PATH%\reports\Textures.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Sounds.html > "%DEPLOY_OUTPUT_PATH%\reports\Sounds.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Shaders.html > "%DEPLOY_OUTPUT_PATH%\reports\Shaders.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Meshes.html > "%DEPLOY_OUTPUT_PATH%\reports\Meshes.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Index.html > "%DEPLOY_OUTPUT_PATH%\reports\Index.html"

) else if "%1"=="deploy" (

	%TRAKTOR_HOME%bin\win32\CeDeploy "\Program Files\%DEPLOY_PROJECT_NAME%" %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\mobile6\releaseshared\*.dll > "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	%TRAKTOR_HOME%bin\win32\CeDeploy "\Program Files\%DEPLOY_PROJECT_NAME%" %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\mobile6\releaseshared\*.exe >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	%TRAKTOR_HOME%bin\win32\CeDeploy "\Program Files\%DEPLOY_PROJECT_NAME%" Application.config >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"

) else if "%1"=="launch" (

	%TRAKTOR_HOME%bin\win32\CeLaunch "\Program Files\%DEPLOY_PROJECT_NAME%\%DEPLOY_EXECUTABLE%" Application.config > "%DEPLOY_OUTPUT_PATH%\Launch.log"
	
) else if "%1"=="migrate" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Database.Migrate.App %DEPLOY_SOURCE_CS% %DEPLOY_TARGET_CS% %DEPLOY_MODULES% > "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	
)
