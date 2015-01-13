#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../../config.sh"

# Build solution files.
$TRAKTOR_HOME/bin/linux/SolutionBuilder -f=cblocks RunLinux.xms

