#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

SCRATCH_PATH="$HOME/Library/Doctor Entertainment AB/TraktorRemoteServer"

mkdir -p "$SCRATCH_PATH"
$TRAKTOR_HOME/bin/linux/RemoteServer "$SCRATCH_PATH"
