#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Ensure correct path and build solution files.
$TRAKTOR_HOME/bin/linux/SolutionBuilder \
	-f=cblocks \
	-w=$TRAKTOR_HOME/resources/build/configurations/cblocks-linux-gcc-workspace.sb \
	-p=$TRAKTOR_HOME/resources/build/configurations/cblocks-linux-gcc-project.sb \
	$TRAKTOR_HOME/resources/build/TraktorLinux.xms
