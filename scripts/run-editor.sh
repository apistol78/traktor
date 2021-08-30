#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"
. "`dirname \"$BASH_SOURCE\"`/config-android.sh"

# Update all binaries.
export TC_HOST=http://pn-tambor-ws.pistol.one:8111

$RUN $TRAKTOR_HOME/scripts/runtime/targets/sync-teamcity-latest.run $TC_HOST $TRAKTOR_HOME/bin/latest/android Traktor_Android
$RUN $TRAKTOR_HOME/scripts/runtime/targets/sync-teamcity-latest.run $TC_HOST $TRAKTOR_HOME/bin/latest/ios Traktor_IOS
$RUN $TRAKTOR_HOME/scripts/runtime/targets/sync-teamcity-latest.run $TC_HOST $TRAKTOR_HOME/bin/latest/linux Traktor_Linux
$RUN $TRAKTOR_HOME/scripts/runtime/targets/sync-teamcity-latest.run $TC_HOST $TRAKTOR_HOME/bin/latest/osx Traktor_OSX
$RUN $TRAKTOR_HOME/scripts/runtime/targets/sync-teamcity-latest.run $TC_HOST $TRAKTOR_HOME/bin/latest/rpi Traktor_RaspberryPi
$RUN $TRAKTOR_HOME/scripts/runtime/targets/sync-teamcity-latest.run $TC_HOST $TRAKTOR_HOME/bin/latest/win64 Traktor_Win64

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

