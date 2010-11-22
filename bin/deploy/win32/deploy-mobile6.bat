@echo off

if "%1"=="build" (

	%DEPLOY_PROJECTROOT%\bin\latest\win32\releaseshared\Traktor.Pipeline.App -p -s=Traktor.Pipeline -l=Pipeline.log %2

) else if "%1"=="deploy" (

	%TRAKTOR_HOME%bin\CeDeploy "\Program Files\Amalgam" $(TRAKTOR_HOME)/build/mobile6/releaseshared/*.dll
	%TRAKTOR_HOME%bin\CeDeploy "\Program Files\Amalgam" $(AMALGAM_HOME)/build/mobile6/releaseshared/*.dll
	%TRAKTOR_HOME%bin\CeDeploy "\Program Files\Amalgam" $(AMALGAM_HOME)/build/mobile6/releaseshared/*.exe
	%TRAKTOR_HOME%bin\CeDeploy "\Program Files\Amalgam" Application.config

) else if "%1"=="launch" (

	%TRAKTOR_HOME%bin\CeLaunch "\Program Files\Amalgam\Amalgam.Viewer" Application.config
	
) else (
	echo "Usage: [deploy] command (args...)"
)
