@echo off

if "%1"=="build" (

	%DEPLOY_PROJECT_ROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2 %3 %4 %5

) else if "%1"=="deploy" (

	rem Deploy Emscripten linkage script.
	pushd %TRAKTOR_HOME%\bin\win32
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% link-emscripten.bat > "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	popd
	
	rem Deploy Emscripten binaries.
	pushd %DEPLOY_PROJECT_ROOT:/=\%\bin\latest\emscripten\releasestatic
	%TRAKTOR_HOME%\bin\win32\RemoteDeploy %DEPLOY_TARGET_HOST% *.lib >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	popd
	
	rem Link Emscripten binary.
	%TRAKTOR_HOME%\bin\win32\RemoteLaunch -w %DEPLOY_TARGET_HOST% link-emscripten.bat "%DEPLOY_EXECUTABLE% %DEPLOY_MODULES%" >> "%DEPLOY_OUTPUT_PATH%\Deploy.log"
	
) else if "%1"=="launch" (

) else if "%1"=="migrate" (

)
