#!/bin/sh

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

SCRATCH_PATH="$HOME/Library/Doctor Entertainment AB/TraktorRemoteServer"

mkdir "$SCRATCH_PATH"
$TRAKTOR_HOME/bin/osx/RemoteServer "$SCRATCH_PATH"
