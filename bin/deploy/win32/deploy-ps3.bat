@echo off

if "%1"=="build" (

	%DEPLOY_PROJECTROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2

) else if "%1"=="deploy" (

	if not exist bin ( mkdir bin )
	xcopy /Q /R /Y %DEPLOY_PROJECTROOT:/=\%\bin\latest\ps3\releasestatic\*.self bin
	
) else if "%1"=="launch" (

	ps3run -k -r -f . bin/%DEPLOY_EXECUTABLE%.self Application.config
	
)
