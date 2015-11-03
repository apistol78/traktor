#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Ensure correct path and build solution files.
$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=cblocks -w=$TRAKTOR_HOME/bin/cblocks-osx-clang-workspace.sb -p=$TRAKTOR_HOME/bin/cblocks-osx-clang-project.sb TraktorOSX.xms

