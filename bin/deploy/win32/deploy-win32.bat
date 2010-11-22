@echo off

if "%1"=="build" (

	%DEPLOY_PROJECTROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Traktor.Pipeline -l=Pipeline.log %2

) else if "%1"=="deploy" (

	if not exist target ( mkdir target )

	xcopy /Q /R /Y %DEPLOY_PROJECTROOT:/=\%\bin\latest\win32\releaseshared\*.dll target
	xcopy /Q /R /Y %DEPLOY_PROJECTROOT:/=\%\bin\latest\win32\releaseshared\*.exe target

) else if "%1"=="launch" (

	target\%DEPLOY_EXECUTABLE% -s Application.config

) else (
	echo "Usage: [deploy] command (args...)"
)
