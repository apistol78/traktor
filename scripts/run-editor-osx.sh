#!/bin/sh

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"
. "`dirname \"$BASH_SOURCE\"`/config-android.sh"

# Launch editor.
$TRAKTOR_HOME/bin/latest/osx/releaseshared/Traktor.Editor.App
