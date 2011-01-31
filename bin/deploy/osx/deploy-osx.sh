#!/bin/sh

if [[ ${1} == "build" ]]; then

	$DEPLOY_PROJECTROOT/bin/latest/osx/releaseshared/Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log ${2}

elif [[ ${1} == "deploy" ]]; then

	mkdir bin
	cp -Rf $DEPLOY_PROJECTROOT/bin/latest/osx/releaseshared/*.dylib bin/
	cp -Rf $DEPLOY_PROJECTROOT/bin/latest/osx/releaseshared/*.app bin/

elif [[ ${1} == "launch" ]]; then

	open bin/$DEPLOY_EXECUTABLE.app --args "$PWD/Application.config"

fi
