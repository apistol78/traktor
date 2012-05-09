#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Ensure correct path and build solution files.
cd $TRAKTOR_HOME
wine $TRAKTOR_HOME/bin/win32/SolutionBuilder -f=cblocks TraktorLinux.xms

