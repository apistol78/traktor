#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

export SOLUTIONBUILDER="$TRAKTOR_HOME/bin/latest/osx/releaseshared/Traktor.SolutionBuilder.App"

# Ensure correct path and build solution files.
pushd $TRAKTOR_HOME

$SOLUTIONBUILDER -f=xcode -xcode-workspace-template=$TRAKTOR_HOME/resources/build/configurations/xcode-workspace.sb -xcode-project-template=$TRAKTOR_HOME/resources/build/configurations/xcode-project.sb $TRAKTOR_HOME/resources/build/ExternOSX.xms
$SOLUTIONBUILDER -f=xcode -xcode-workspace-template=$TRAKTOR_HOME/resources/build/configurations/xcode-workspace.sb -xcode-project-template=$TRAKTOR_HOME/resources/build/configurations/xcode-project.sb $TRAKTOR_HOME/resources/build/TraktorOSX.xms

popd

