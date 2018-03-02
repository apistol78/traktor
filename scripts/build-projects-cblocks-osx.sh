#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Ensure correct path and build solution files.
$TRAKTOR_HOME/bin/osx/SolutionBuilder \
	-f=cblocks \
	-w=$TRAKTOR_HOME/resources/build/configurations/cblocks-osx-clang-workspace.sb \
	-p=$TRAKTOR_HOME/resources/build/configurations/cblocks-osx-clang-project.sb \
	$TRAKTOR_HOME/resources/build/TraktorOSX.xms

