#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Launch editor.
($TRAKTOR_HOME/bin/latest/rpi/releaseshared/Traktor.Editor.App > /dev/null &)
