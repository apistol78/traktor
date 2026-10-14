#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Determine platform from some simple heuristic.
if [[ `uname -s` == Linux* ]]; then
	if [[ `uname -m` == arm* ]]; then
		export TRAKTOR_PLATFORM=rpi
	else
		export TRAKTOR_PLATFORM=linux
	fi
elif [[ `uname -s` == Darwin* ]]; then
	export TRAKTOR_PLATFORM=macos
fi

# Launch editor.
if [[ $TRAKTOR_PLATFORM == linux ]]; then
	($TRAKTOR_HOME/bin/latest/linux/releaseshared/Traktor.Editor.App > /dev/null &)
elif [[ $TRAKTOR_PLATFORM == rpi ]]; then
	($TRAKTOR_HOME/bin/latest/rpi/releaseshared/Traktor.Editor.App > /dev/null &)
elif [[ $TRAKTOR_PLATFORM == macos ]]; then
	($TRAKTOR_HOME/bin/latest/osx/releaseshared/Traktor.Editor.App > /dev/null &)
else
	echo "Unknown platform."
fi
