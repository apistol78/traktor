#!/bin/bash

# Ensure terminal output is cleared.
clear

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Build
if [[ `uname -s` == Linux* ]]; then
	pushd $TRAKTOR_HOME/build/linux
	make -s -j6 -f 'Extern Linux.mak' $1
	make -s -j6 -f 'Traktor Linux.mak' $1
	popd
elif [[ `uname -s` == Darwin* ]]; then
	pushd $TRAKTOR_HOME/build/osx
	make -s -j6 -f 'Extern OSX.mak' $1
	make -s -j6 -f 'Traktor OSX.mak' $1
	popd
fi
