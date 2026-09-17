#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../../scripts/config.sh"

# The Traktor shared libraries this example links against are aggregated
# alongside it, so running from there needs no library path of its own.
exec $TRAKTOR_HOME/bin/latest/$TRAKTOR_PLATFORM/releaseshared/Llm.App "$@"
