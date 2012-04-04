@echo off

set BUNDLE=%DEPLOY_PROJECT_NAME: =%

if "%1"=="build" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2 %3 %4 %5

) else if "%1"=="deploy" (

	rem Deploy iPad simulator launcher.
	pushd %TRAKTOR_HOME%\bin\osx
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% waxsim > %DEPLOY_PROJECT_ROOT%\deploy.log
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% launch-ipad.sh >> %DEPLOY_PROJECT_ROOT%\deploy.log
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% link-ipad.sh >> %DEPLOY_PROJECT_ROOT%\deploy.log
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% convert-icon-ipad.sh >> %DEPLOY_PROJECT_ROOT%\deploy.log
	popd

	rem Deploy iPad resources.
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\iPad\Info.plist .
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\iPad\Default.png Default.png
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\%DEPLOY_PROJECT_ICON:/=\% Icon.png
	%TRAKTOR_HOME%\bin\Replace -e=utf8 Info.plist "$DEPLOY_PROJECT_NAME" "%DEPLOY_PROJECT_NAME%" "$DEPLOY_EXECUTABLE" "%DEPLOY_EXECUTABLE%" "$DEPLOY_PROJECT_IDENTIFIER" "%DEPLOY_PROJECT_IDENTIFIER%" >> %DEPLOY_PROJECT_ROOT%\deploy.log
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% *.plist >> %DEPLOY_PROJECT_ROOT%\deploy.log
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% *.png >> %DEPLOY_PROJECT_ROOT%\deploy.log	

	rem Deploy iPad binaries.
	pushd %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\ipad-simulator\releasestatic
	%TRAKTOR_HOME%\bin\RemoteDeploy -recursive %DEPLOY_TARGET_HOST% *.a >> %DEPLOY_PROJECT_ROOT%\deploy.log
	popd

	rem Build iPad executable.
	%TRAKTOR_HOME%\bin\RemoteLaunch -w %DEPLOY_TARGET_HOST% link-ipad.sh "%BUNDLE% %DEPLOY_EXECUTABLE% %DEPLOY_MODULES%" >> %DEPLOY_PROJECT_ROOT%\deploy.log
	%TRAKTOR_HOME%\bin\RemoteLaunch -w %DEPLOY_TARGET_HOST% convert-icon-ipad.sh "Icon.png Default.png %BUNDLE%.app" >> %DEPLOY_PROJECT_ROOT%\deploy.log

) else if "%1"=="launch" (

	rem Launch application with simulator launcher on target.
	%TRAKTOR_HOME%\bin\RemoteDeploy -target-base="%BUNDLE%.app" %DEPLOY_TARGET_HOST% Application.config > %DEPLOY_PROJECT_ROOT%\launch.log
	%TRAKTOR_HOME%\bin\RemoteLaunch %DEPLOY_TARGET_HOST% launch-ipad.sh "%BUNDLE%.app" >> %DEPLOY_PROJECT_ROOT%\launch.log

) else if "%1"=="migrate" (

	rem Migrate into compact database.
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Database.Migrate.App %DEPLOY_SOURCE_CS% %DEPLOY_TARGET_CS% %DEPLOY_MIGRATE_MODULES% > %DEPLOY_PROJECT_ROOT%\migrate.log

	rem Deploy iPad build scripts.
	pushd %TRAKTOR_HOME%\bin\osx
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% link-device-ipad.sh >> %DEPLOY_PROJECT_ROOT%\migrate.log
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% create-ipa-ipad.sh >> %DEPLOY_PROJECT_ROOT%\migrate.log
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% convert-icon-ipad.sh >> %DEPLOY_PROJECT_ROOT%\migrate.log
	popd

	rem Deploy iPad resources.
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\iPad\Info.plist .
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\res\deploy\iPad\Default.png Default.png
	copy /Y %DEPLOY_PROJECT_ROOT:/=\%\%DEPLOY_PROJECT_ICON:/=\% Icon.png
	%TRAKTOR_HOME%\bin\Replace -e=utf8 Info.plist "$DEPLOY_PROJECT_NAME" "%DEPLOY_PROJECT_NAME%" "$DEPLOY_EXECUTABLE" "%DEPLOY_EXECUTABLE%" "$DEPLOY_PROJECT_IDENTIFIER" "%DEPLOY_PROJECT_IDENTIFIER%" >> %DEPLOY_PROJECT_ROOT%\migrate.log
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% *.plist >> %DEPLOY_PROJECT_ROOT%\migrate.log
	%TRAKTOR_HOME%\bin\RemoteDeploy %DEPLOY_TARGET_HOST% *.png >> %DEPLOY_PROJECT_ROOT%\migrate.log	

	rem Deploy iPad binaries.
	pushd %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\ipad\releasestatic
	%TRAKTOR_HOME%\bin\RemoteDeploy -recursive %DEPLOY_TARGET_HOST% *.a >> %DEPLOY_PROJECT_ROOT%\migrate.log
	popd

	rem Build iPad executable.
	%TRAKTOR_HOME%\bin\RemoteLaunch -w %DEPLOY_TARGET_HOST% link-device-ipad.sh "%BUNDLE% %DEPLOY_EXECUTABLE% %DEPLOY_RUNTIME_MODULES%" >> %DEPLOY_PROJECT_ROOT%\migrate.log
	%TRAKTOR_HOME%\bin\RemoteLaunch -w %DEPLOY_TARGET_HOST% convert-icon-ipad.sh "Icon.png Default.png %BUNDLE%.app" >> %DEPLOY_PROJECT_ROOT%\deploy.log

	rem Deploy files into bundle.
	%TRAKTOR_HOME%\bin\RemoteDeploy -target-base="%BUNDLE%.app" %DEPLOY_TARGET_HOST% "Application.config" > %DEPLOY_PROJECT_ROOT%\migrate.log
	%TRAKTOR_HOME%\bin\RemoteDeploy -target-base="%BUNDLE%.app" %DEPLOY_TARGET_HOST% "Content.compact" >> %DEPLOY_PROJECT_ROOT%\migrate.log

	rem Deploy mobile provision file into bundle.
	pushd %DEPLOY_PROJECT_ROOT:/=\%\res\iPad
	%TRAKTOR_HOME%\bin\RemoteDeploy -target-base="%BUNDLE%.app" %DEPLOY_TARGET_HOST% "embedded.mobileprovision" >> %DEPLOY_PROJECT_ROOT%\migrate.log
	%TRAKTOR_HOME%\bin\RemoteDeploy -target-base="%BUNDLE%.app" %DEPLOY_TARGET_HOST% "ResourceRules.plist" >> %DEPLOY_PROJECT_ROOT%\migrate.log
	popd

	rem Sign and create IPA from bundle.
	%TRAKTOR_HOME%\bin\RemoteLaunch -w %DEPLOY_TARGET_HOST% create-ipa-ipad.sh "%BUNDLE%" >> %DEPLOY_PROJECT_ROOT%\migrate.log

)
