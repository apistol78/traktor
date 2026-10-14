#!/bin/bash

# Get normalized path to this script, excluding file name.
if [[ $BASH_SOURCE != "" ]]; then
	SCRIPT=$BASH_SOURCE
else
	SCRIPT=$0
fi
TRAKTOR_HOME="`dirname \"$SCRIPT\"`/.."
TRAKTOR_HOME="`(cd \"$TRAKTOR_HOME\" && pwd)`"
export TRAKTOR_HOME
