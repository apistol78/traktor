#!/bin/sh

if [[ ${1} == "build" ]]; then

	$DEPLOY_PROJECT_ROOT/bin/latest/osx/releaseshared/Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log ${2} ${3} ${4} ${5}

elif [[ ${1} == "deploy" ]]; then

	pushd $DEPLOY_PROJECT_ROOT/bin/latest/osx/releaseshared
	$TRAKTOR_HOME/bin/osx/RemoteDeploy $DEPLOY_TARGET_HOST * > $DEPLOY_PROJECT_ROOT/deploy.log
	popd

elif [[ ${1} == "launch" ]]; then

	$TRAKTOR_HOME/bin/osx/RemoteDeploy $DEPLOY_TARGET_HOST Application.config > $DEPLOY_PROJECT_ROOT/deploy.log
	$TRAKTOR_HOME/bin/osx/RemoteLaunch $DEPLOY_TARGET_HOST $DEPLOY_EXECUTABLE "\-s Application.config" >> $DEPLOY_PROJECT_ROOT/deploy.log

fi
