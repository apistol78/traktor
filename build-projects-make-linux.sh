#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Build solution files.
$TRAKTOR_HOME/bin/linux/SolutionBuilder -f=make -make-dialect=gnu -make-platform=linux TraktorLinux.xms

