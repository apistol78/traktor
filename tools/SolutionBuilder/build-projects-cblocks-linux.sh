#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../../config.sh"

# Ensure correct path and build solution files.
$TRAKTOR_HOME/bin/linux/SolutionBuilder -f=cblocks SolutionBuilderLinux.xms

