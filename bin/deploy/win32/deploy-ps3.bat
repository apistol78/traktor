@echo off

if "%1"=="build" (

	%DEPLOY_PROJECTROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Traktor.Pipeline -l=Pipeline.log %2

) else if "%1"=="deploy" (

	if not exist target ( mkdir target )
	
	xcopy /Q /R /Y %DEPLOY_PROJECTROOT:/=\%\bin\latest\ps3\releasestatic\*.self target
	
) else if "%1"=="launch" (

	xcopy /Q /Y Application.config target

	pushd target
	ps3run -k -r -f . %DEPLOY_EXECUTABLE% Application.config
	popd
	
) else (
	echo "Usage: [deploy] command (args...)"
)
