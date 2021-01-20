#!/bin/sh

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Launch remote server.
$TRAKTOR_HOME/bin/latest/osx/releaseshared/Traktor.Remote.Server.App "$TRAKTOR_HOME\scripts\remote\Server.run" "$HOME/Library/Traktor/Editor/Remote"
