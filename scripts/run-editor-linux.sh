#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Launch remote server.
$TRAKTOR_HOME/bin/latest/linux/releaseshared/Traktor.Remote.Server.App "$HOME/Library/Doctor Entertainment AB/TraktorRemoteServer" &

# Launch editor.
($TRAKTOR_HOME/bin/latest/linux/releaseshared/Traktor.Editor.App > /dev/null &)
