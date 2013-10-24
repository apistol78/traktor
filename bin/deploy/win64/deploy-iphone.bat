@echo off

set BUNDLE=%DEPLOY_PROJECT_NAME: =%

if "%1"=="build" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win64\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2 %3 %4 %5

	if not exist "%DEPLOY_OUTPUT_PATH%\reports" mkdir "%DEPLOY_OUTPUT_PATH%\reports"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win64\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Textures.html > "%DEPLOY_OUTPUT_PATH%\reports\Textures.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win64\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Sounds.html > "%DEPLOY_OUTPUT_PATH%\reports\Sounds.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win64\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Shaders.html > "%DEPLOY_OUTPUT_PATH%\reports\Shaders.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win64\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Meshes.html > "%DEPLOY_OUTPUT_PATH%\reports\Meshes.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win64\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Index.html > "%DEPLOY_OUTPUT_PATH%\reports\Index.html"

) else if "%1"=="deploy" (

	rem Deploy iPhone simulator launcher.
	pushd %TRAKTOR_HOME%\bin\osx
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% waxsim > "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% launch-iphone.sh >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% link-iphone.sh >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% convert-icon-iphone.sh >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	popd

	rem Deploy iPhone resources.
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\iPhone\Info.plist .
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\iPhone\Default.png Default.png
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\%DEPLOY_PROJECT_ICON:/=\% Icon.png
	%TRAKTOR_HOME%\bin\win32\Replace -e=utf8 Info.plist "$DEPLOY_PROJECT_NAME" "%DEPLOY_PROJECT_NAME%" "$DEPLOY_EXECUTABLE" "%DEPLOY_EXECUTABLE%" "$DEPLOY_PROJECT_IDENTIFIER" "%DEPLOY_PROJECT_IDENTIFIER%" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% *.plist >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% *.png >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"

	rem Deploy iPhone binaries.
	pushd %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\iphone-simulator\releasestatic
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -recursive %DEPLOY_TARGET_HOST% *.a >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	popd

	rem Build iPhone executable.
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch -w %DEPLOY_TARGET_HOST% link-iphone.sh "%BUNDLE% %DEPLOY_EXECUTABLE% %DEPLOY_MODULES%" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch -w %DEPLOY_TARGET_HOST% convert-icon-iphone.sh "Icon.png Default.png %BUNDLE%.app" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	
) else if "%1"=="launch" (

	rem Launch application with simulator launcher on target.
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -target-base="%BUNDLE%.app" %DEPLOY_TARGET_HOST% Application.config > "%DEPLOY_OUTPUT_PATH%\Launch.log"
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch %DEPLOY_TARGET_HOST% launch-iphone.sh "%BUNDLE%.app" >> "%DEPLOY_OUTPUT_PATH%\Launch.log"

) else if "%1"=="migrate" (

	rem Migrate into compact database.
	%DEPLOY_PROJECT_ROOT%\bin\latest\win64\releaseshared\Traktor.Database.Migrate.App %DEPLOY_SOURCE_CS% %DEPLOY_TARGET_CS% %DEPLOY_MIGRATE_MODULES% > "%DEPLOY_OUTPUT_PATH%\Migrate.log"

	rem Deploy iPhone build scripts.
	pushd %TRAKTOR_HOME%\bin\osx
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% link-device-iphone.sh >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% create-ipa-iphone.sh >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% convert-icon-iphone.sh >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	popd

	rem Deploy iPhone resources.
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\iPhone\Info.plist .
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\iPhone\Default.png Default.png
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\%DEPLOY_PROJECT_ICON:/=\% Icon.png
	%TRAKTOR_HOME%\bin\win32\Replace -e=utf8 Info.plist "$DEPLOY_PROJECT_NAME" "%DEPLOY_PROJECT_NAME%" "$DEPLOY_EXECUTABLE" "%DEPLOY_EXECUTABLE%" "$DEPLOY_PROJECT_IDENTIFIER" "%DEPLOY_PROJECT_IDENTIFIER%" >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% *.plist >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% *.png >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"

	rem Deploy iPhone binaries.
	pushd %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\iphone\releasestatic
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -recursive %DEPLOY_TARGET_HOST% *.a >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	popd

	rem Build iPhone executable.
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch -w %DEPLOY_TARGET_HOST% link-device-iphone.sh "%BUNDLE% %DEPLOY_EXECUTABLE% %DEPLOY_RUNTIME_MODULES%" >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch -w %DEPLOY_TARGET_HOST% convert-icon-iphone.sh "Icon.png Default.png %BUNDLE%.app" >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"

	rem Deploy files into bundle.
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -target-base="%BUNDLE%.app" %DEPLOY_TARGET_HOST% "Application.config" >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -target-base="%BUNDLE%.app" %DEPLOY_TARGET_HOST% "Content.compact" >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"

	rem Deploy mobile provision file into bundle.
	pushd %DEPLOY_PROJECT_ROOT:/=\%\res\iphone
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -target-base="%BUNDLE%.app" %DEPLOY_TARGET_HOST% "embedded.mobileprovision" >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -target-base="%BUNDLE%.app" %DEPLOY_TARGET_HOST% "ResourceRules.plist" >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"
	popd

	rem Sign and create IPA from bundle.
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch -w %DEPLOY_TARGET_HOST% create-ipa-iphone.sh "%BUNDLE%" >> "%DEPLOY_OUTPUT_PATH%\Migrate.log"

)
