#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Launch remote server.
if [[ $TRAKTOR_PLATFORM == linux ]]; then
	$TRAKTOR_HOME/bin/latest/linux/releaseshared/Traktor.Run.App "$TRAKTOR_HOME\scripts\remote\Server.run" "$HOME/.traktor/Traktor/Editor/Remote"
elif [[ $TRAKTOR_PLATFORM == rpi ]]; then
	$TRAKTOR_HOME/bin/latest/rpi/releaseshared/Traktor.Run.App "$TRAKTOR_HOME\scripts\remote\Server.run" "$HOME/.traktor/Traktor/Editor/Remote"
elif [[ $TRAKTOR_PLATFORM == macos ]]; then
	$TRAKTOR_HOME/bin/latest/osx/releaseshared/Traktor.Run.App "$TRAKTOR_HOME\scripts\remote\Server.run" "$HOME/Library/Traktor/Editor/Remote"
else
	echo "Unknown platform."
fi
