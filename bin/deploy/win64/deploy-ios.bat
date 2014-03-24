@echo off

:: -- note --
:: DEPLOY_IOS_KIND is defined as "iphone" or "ipad"

set BUNDLE=%DEPLOY_PROJECT_NAME: =%
set LOGFILE="%DEPLOY_OUTPUT_PATH%\%1.log"

if exist "%LOGFILE%" del "%LOGFILE%"

if "%1"=="build" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win64\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2 %3 %4 %5

	if not exist "%DEPLOY_OUTPUT_PATH%\reports" mkdir "%DEPLOY_OUTPUT_PATH%\reports"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win64\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Textures.html > "%DEPLOY_OUTPUT_PATH%\reports\Textures.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win64\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Sounds.html > "%DEPLOY_OUTPUT_PATH%\reports\Sounds.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win64\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Shaders.html > "%DEPLOY_OUTPUT_PATH%\reports\Shaders.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win64\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Meshes.html > "%DEPLOY_OUTPUT_PATH%\reports\Meshes.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win64\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Index.html > "%DEPLOY_OUTPUT_PATH%\reports\Index.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win64\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/PhysMeshes.html > "%DEPLOY_OUTPUT_PATH%\reports\PhysMeshes.html"

) else if "%1"=="deploy" (

	:: Deploy iOS target tools.
	pushd %TRAKTOR_HOME%\bin\osx
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% ios-deploy >> "%LOGFILE%"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% launch-ios-device.sh >> "%LOGFILE%"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% link-ios-device.sh >> "%LOGFILE%"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% convert-icon.sh >> "%LOGFILE%"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% sign-ios.sh >> "%LOGFILE%"
	popd

	:: Deploy iOS resources.
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\ios\Info.plist .
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\ios\Default.png .
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\%DEPLOY_PROJECT_ICON:/=\% Icon.png
	%TRAKTOR_HOME%\bin\win32\Replace -e=utf8 Info.plist "$DEPLOY_PROJECT_NAME" "%DEPLOY_PROJECT_NAME%" "$DEPLOY_EXECUTABLE" "%DEPLOY_EXECUTABLE%" "$DEPLOY_PROJECT_IDENTIFIER" "%DEPLOY_PROJECT_IDENTIFIER%" >> "%LOGFILE%"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% *.plist >> "%LOGFILE%"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% *.png >> "%LOGFILE%"

	:: Deploy iOS binaries.
	pushd %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\ios\releasestatic
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -recursive %DEPLOY_TARGET_HOST% *.a >> "%LOGFILE%"
	popd

	:: Build iOS executable and bundle.
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch -w %DEPLOY_TARGET_HOST% link-ios-device.sh "%BUNDLE% %DEPLOY_EXECUTABLE% %DEPLOY_MODULES%" >> "%LOGFILE%"
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch -w %DEPLOY_TARGET_HOST% convert-icon.sh "Icon.png Default.png %BUNDLE%.app" >> "%LOGFILE%"

) else if "%1"=="launch" (

	:: Deploy modifiable files into bundle.
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -target-base="%BUNDLE%.app" %DEPLOY_TARGET_HOST% "Application.config" >> "%LOGFILE%"

	:: Sign bundle.
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch -w %DEPLOY_TARGET_HOST% sign-ios.sh "%BUNDLE%" >> "%LOGFILE%"

	:: Launch application from target.
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch %DEPLOY_TARGET_HOST% launch-ios-device.sh "%BUNDLE%.app" >> "%LOGFILE%"

) else if "%1"=="migrate" (

	:: Migrate into compact database.
	%DEPLOY_PROJECT_ROOT%\bin\latest\win64\releaseshared\Traktor.Database.Migrate.App %DEPLOY_SOURCE_CS% %DEPLOY_TARGET_CS% %DEPLOY_MIGRATE_MODULES% >> "%LOGFILE%"

	:: Deploy iOS build scripts.
	pushd %TRAKTOR_HOME%\bin\osx
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% link-ios-device.sh >> "%LOGFILE%"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% create-ipa.sh >> "%LOGFILE%"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% convert-icon.sh >> "%LOGFILE%"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% sign-ios.sh >> "%LOGFILE%"
	popd

	:: Deploy iOS resources.
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\ios\Info.plist .
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\ios\Default.png Default.png
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\%DEPLOY_PROJECT_ICON:/=\% Icon.png
	%TRAKTOR_HOME%\bin\win32\Replace -e=utf8 Info.plist "$DEPLOY_PROJECT_NAME" "%DEPLOY_PROJECT_NAME%" "$DEPLOY_EXECUTABLE" "%DEPLOY_EXECUTABLE%" "$DEPLOY_PROJECT_IDENTIFIER" "%DEPLOY_PROJECT_IDENTIFIER%" >> "%LOGFILE%"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% *.plist >> "%LOGFILE%"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% *.png >> "%LOGFILE%"

	:: Deploy iOS binaries.
	pushd %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\ios\releasestatic
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -recursive %DEPLOY_TARGET_HOST% *.a >> "%LOGFILE%"
	popd

	:: Build iOS executable.
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch -w %DEPLOY_TARGET_HOST% link-ios-device.sh "%BUNDLE% %DEPLOY_EXECUTABLE% %DEPLOY_RUNTIME_MODULES%" >> "%LOGFILE%"
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch -w %DEPLOY_TARGET_HOST% convert-icon.sh "Icon.png Default.png %BUNDLE%.app" >> "%LOGFILE%"

	:: Deploy files into bundle.
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -target-base="%BUNDLE%.app" %DEPLOY_TARGET_HOST% "Application.config" >> "%LOGFILE%"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -target-base="%BUNDLE%.app" %DEPLOY_TARGET_HOST% "Content.compact" >> "%LOGFILE%"

	:: Deploy mobile provision file into bundle.
	pushd %DEPLOY_PROJECT_ROOT:/=\%\res\ios
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -target-base="%BUNDLE%.app" %DEPLOY_TARGET_HOST% "embedded.mobileprovision" >> "%LOGFILE%"
	popd

	:: Sign bundle.
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch -w %DEPLOY_TARGET_HOST% sign-ios.sh "%BUNDLE%" >> "%LOGFILE%"

	:: Create IPA from bundle.
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch -w %DEPLOY_TARGET_HOST% create-ipa.sh "%BUNDLE%" >> "%LOGFILE%"

)
