#!/bin/sh

if [[ ${1} == "build" ]]; then

	$DEPLOY_PROJECTROOT/bin/latest/osx/releaseshared/Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log ${2}

elif [[ ${1} == "deploy" ]]; then

	pushd $DEPLOY_PROJECTROOT/bin/latest/osx/releaseshared
	$TRAKTOR_HOME/bin/osx/RemoteDeploy $DEPLOY_TARGET_HOST * > $DEPLOY_PROJECTROOT/deploy.log
	popd

elif [[ ${1} == "launch" ]]; then

	$TRAKTOR_HOME/bin/osx/RemoteDeploy $DEPLOY_TARGET_HOST Application.config > $DEPLOY_PROJECTROOT/deploy.log
	$TRAKTOR_HOME/bin/osx/RemoteLaunch $DEPLOY_TARGET_HOST $DEPLOY_EXECUTABLE "\-s Application.config" >> $DEPLOY_PROJECTROOT/deploy.log

fi
