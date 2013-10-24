@echo off

set BUNDLE=%DEPLOY_PROJECT_NAME: =%

if "%1"=="build" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2 %3 %4 %5

	if not exist "%DEPLOY_OUTPUT_PATH%\reports" mkdir "%DEPLOY_OUTPUT_PATH%\reports"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Textures.html > "%DEPLOY_OUTPUT_PATH%\reports\Textures.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Sounds.html > "%DEPLOY_OUTPUT_PATH%\reports\Sounds.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Shaders.html > "%DEPLOY_OUTPUT_PATH%\reports\Shaders.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Meshes.html > "%DEPLOY_OUTPUT_PATH%\reports\Meshes.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Index.html > "%DEPLOY_OUTPUT_PATH%\reports\Index.html"

) else if "%1"=="deploy" (

	%TRAKTOR_HOME%\bin\win32\link-pnacl.bat %DEPLOY_EXECUTABLE% > "%DEPLOY_OUTPUT_PATH%\Deploy.log"

	if not exist "%BUNDLE%" mkdir "%BUNDLE%"

	copy /Y Application.config "%BUNDLE%" >> %DEPLOY_OUTPUT_PATH%\Deploy.log
	copy /Y %DEPLOY_EXECUTABLE%.pexe "%BUNDLE%" >> %DEPLOY_OUTPUT_PATH%\Deploy.log
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\pnacl\manifest.nmf "%BUNDLE%\%DEPLOY_EXECUTABLE%.nmf" >> %DEPLOY_OUTPUT_PATH%\Deploy.log
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\pnacl\manifest.json "%BUNDLE%" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\pnacl\run-nacl.html "%BUNDLE%" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\pnacl\common.js "%BUNDLE%" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\pnacl\nacl_icon_16.jpg "%BUNDLE%" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\pnacl\nacl_icon_128.jpg "%BUNDLE%" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	
	%TRAKTOR_HOME%\bin\win32\Replace -e=utf8 "%BUNDLE%\manifest.json" "$DEPLOY_PROJECT_NAME" "%DEPLOY_PROJECT_NAME%" "$DEPLOY_EXECUTABLE" "%DEPLOY_EXECUTABLE%" "$DEPLOY_PROJECT_IDENTIFIER" "%DEPLOY_PROJECT_IDENTIFIER%" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	%TRAKTOR_HOME%\bin\win32\Replace -e=utf8 "%BUNDLE%\run-nacl.html" "$DEPLOY_PROJECT_NAME" "%DEPLOY_PROJECT_NAME%" "$DEPLOY_EXECUTABLE" "%DEPLOY_EXECUTABLE%" "$DEPLOY_PROJECT_IDENTIFIER" "%DEPLOY_PROJECT_IDENTIFIER%" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	%TRAKTOR_HOME%\bin\win32\Replace -e=utf8 "%BUNDLE%\%DEPLOY_EXECUTABLE%.nmf" "$DEPLOY_PROJECT_NAME" "%DEPLOY_PROJECT_NAME%" "$DEPLOY_EXECUTABLE" "%DEPLOY_EXECUTABLE%" "$DEPLOY_PROJECT_IDENTIFIER" "%DEPLOY_PROJECT_IDENTIFIER%" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"

) else if "%1"=="launch" (

	%TRAKTOR_HOME%\bin\win32\launch-pnacl.bat %BUNDLE% > "%DEPLOY_OUTPUT_PATH%\Launch.log"

) else if "%1"=="migrate" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Database.Migrate.App -s=Migrate -l=Migrate.log

)
