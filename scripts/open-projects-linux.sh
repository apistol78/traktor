#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

(codeblocks '$TRAKTOR_HOME/build/linux/Traktor Linux.workspace' > /dev/null &)

