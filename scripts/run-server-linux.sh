#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Launch remote server.
$TRAKTOR_HOME/bin/latest/linux/releaseshared/Traktor.Run.App "$TRAKTOR_HOME\scripts\remote\Server.run" "$HOME/.traktor/Traktor/Editor/Remote"
