#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Launch editor.
if [[ $TRAKTOR_PLATFORM == linux ]]; then
	$TRAKTOR_HOME/bin/latest/linux/releaseshared/Traktor.Avalanche.Server.App -d=/var/tmp/avalanche
else
	echo "Unknown platform."
fi
