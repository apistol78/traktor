#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

($TRAKTOR_HOME/bin/latest/linux/releaseshared/Traktor.Editor.App > /dev/null &)


