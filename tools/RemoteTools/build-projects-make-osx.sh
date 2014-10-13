#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../../config.sh"

# Build solution files.
$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=make -make-dialect=gnu -make-platform=osx -make-configuration=$TRAKTOR_HOME/bin/make-config-osx.inc RemoteToolsOSX.xms
