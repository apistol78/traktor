#!/bin/sh

# -- note --
# DEPLOY_IOS_KIND is defined as "iphone" or "ipad"

if [[ ${1} == "build" ]]; then

	# Build content.
	$DEPLOY_PROJECT_ROOT/bin/latest/osx/releaseshared/Traktor.Pipeline.App -p -s=Pipeline -l=Pipeline.log ${2} ${3} ${4} ${5}

elif [[ ${1} == "deploy" ]]; then

	# Deploy iOS simulator launcher.
	pushd $TRAKTOR_HOME/bin/osx
	$TRAKTOR_HOME/bin/osx/RemoteDeploy $DEPLOY_TARGET_HOST waxsim > $DEPLOY_PROJECT_ROOT/deploy.log
	$TRAKTOR_HOME/bin/osx/RemoteDeploy $DEPLOY_TARGET_HOST launch-ios.sh >> $DEPLOY_PROJECT_ROOT/deploy.log
	popd

	# Deploy iOS binaries.
	pushd $DEPLOY_PROJECT_ROOT/bin/latest/ios-simulator/releasestatic
	$TRAKTOR_HOME/bin/osx/RemoteDeploy -recursive $DEPLOY_TARGET_HOST * >> $DEPLOY_PROJECT_ROOT/deploy.log
	popd

elif [[ ${1} == "launch" ]]; then

	# Launch application with simulator launcher on target.
	$TRAKTOR_HOME/bin/osx/RemoteDeploy -target-base="$DEPLOY_EXECUTABLE.app" $DEPLOY_TARGET_HOST Application.config > $DEPLOY_PROJECT_ROOT/deploy.log
	$TRAKTOR_HOME/bin/osx/RemoteLaunch $DEPLOY_TARGET_HOST launch-ios.sh "$DEPLOY_EXECUTABLE.app" >> $DEPLOY_PROJECT_ROOT/deploy.log

fi
