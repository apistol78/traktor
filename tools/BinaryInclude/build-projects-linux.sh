#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../../config.sh"

wine $TRAKTOR_HOME/bin/win32/SolutionBuilder -f=cblocks BinaryIncludeLinux.xms

