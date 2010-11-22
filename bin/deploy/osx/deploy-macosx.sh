#!/bin/sh

if [[ ${1} == "build" ]]; then

	$DEPLOY_PROJECTROOT/bin/latest/osx/releaseshared/Traktor.Pipeline.App -p -s=Traktor.Pipeline

elif [[ ${1} == "deploy" ]]; then

	mkdir target
	cp -Rf $DEPLOY_PROJECTROOT/bin/latest/osx/releaseshared/*.app target/

elif [[ ${1} == "launch" ]]; then

	open target/$DEPLOY_EXECUTABLE --args Application.config

else
	echo "Usage: [deploy] command (args...)"
fi
