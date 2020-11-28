#!/bin/sh

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Launch remote server.
$TRAKTOR_HOME/bin/latest/osx/releaseshared/Traktor.Remote.Server.App -verbose -port=50001 "$HOME/Library/Traktor/Editor/Remote"
