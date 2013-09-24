@echo off

set PACKAGE=%DEPLOY_PROJECT_NAME: =%

if "%1"=="build" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2 %3 %4 %5

	if not exist "%DEPLOY_OUTPUT_PATH%\reports" mkdir "%DEPLOY_OUTPUT_PATH%\reports"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Textures.html > "%DEPLOY_OUTPUT_PATH%\reports\Textures.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Sounds.html > "%DEPLOY_OUTPUT_PATH%\reports\Sounds.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Shaders.html > "%DEPLOY_OUTPUT_PATH%\reports\Shaders.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Meshes.html > "%DEPLOY_OUTPUT_PATH%\reports\Meshes.html"
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.Report.App %TRAKTOR_HOME%/res/deploy/reports/Index.html > "%DEPLOY_OUTPUT_PATH%\reports\Index.html"

) else if "%1"=="deploy" (

	rem Deploy PS3 linkage script.
	pushd %TRAKTOR_HOME%\bin\win32
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% link-ps3.bat > "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% launch-ps3.bat >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	popd

	rem Deploy PS3 binaries.
	pushd %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\ps3\releasestatic
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -recursive -target-base="ppu" %DEPLOY_TARGET_HOST% *.lib >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	popd
	pushd %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\ps3-spu\releasestatic
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy -recursive -target-base="spu" %DEPLOY_TARGET_HOST% *.lib >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	popd

	rem Link PS3 binary.
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch -w %DEPLOY_TARGET_HOST% link-ps3.bat "%PACKAGE% %DEPLOY_EXECUTABLE% %DEPLOY_MODULES%" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"

) else if "%1"=="launch" (

	rem Deploy configuration and then launch PS3 binary.
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% Application.config > %DEPLOY_OUTPUT_PATH%\Launch.log
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch -w %DEPLOY_TARGET_HOST% launch-ps3.bat "%DEPLOY_EXECUTABLE%.self Application.config" >> "%DEPLOY_OUTPUT_PATH%\Launch.log"

) else if "%1"=="migrate" (

	rem Migrate database.
	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Database.Migrate.App -s=Migrate -l=Migrate.log

)
