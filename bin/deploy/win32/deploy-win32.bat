@echo off

if "%1"=="build" (

	%DEPLOY_PROJECTROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log %2

) else if "%1"=="deploy" (

	if not exist bin ( mkdir bin )
	xcopy /Q /R /Y %DEPLOY_PROJECTROOT:/=\%\bin\latest\win32\releaseshared\*.dll bin\
	xcopy /Q /R /Y %DEPLOY_PROJECTROOT:/=\%\bin\latest\win32\releaseshared\*.exe bin\
	set ERRORLEVEL=0
	
) else if "%1"=="launch" (

	bin\%DEPLOY_EXECUTABLE% -s Application.config
	
)
