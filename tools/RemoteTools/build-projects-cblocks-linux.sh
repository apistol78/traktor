#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../../config.sh"

$TRAKTOR_HOME/bin/linux/SolutionBuilder -f=cblocks RemoteToolsLinux.xms
