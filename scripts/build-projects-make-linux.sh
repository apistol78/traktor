#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Generator configuration.
export MAKE_DIALECT=make
export MAKE_TOOLCHAIN=gcc
export MAKE_OS=linux
export MAKE_INCLUDE="\$(TRAKTOR_HOME)/resources/build/configurations/make-config-linux.inc"
export MAKE_OBJECT_FILE="%s.o"
export MAKE_STATIC_LIBRARY_FILE="lib%s.a"
export MAKE_SHARED_LIBRARY_FILE="lib%s.so"
export MAKE_EXECUTABLE_FILE="%s"

if [ "$AGGREGATE_OUTPUT_PATH" == "" ]; then
	export AGGREGATE_OUTPUT_PATH="\$(TRAKTOR_HOME)/bin/latest/linux"
fi

# Steam redistributable aggregate.
if [ "`uname -m`" == "x86_64" ]; then
	export STEAMWORKS_SDK_BIN="$STEAMWORKS_SDK/redistributable_bin/linux64"
else
	export STEAMWORKS_SDK_BIN="$STEAMWORKS_SDK/redistributable_bin/linux32"
fi

# Build solution files.
$SOLUTIONBUILDER \
	-f=make2 \
	-make-solution-template=$TRAKTOR_HOME/resources/build/configurations/make-solution.sbm \
	-make-project-template=$TRAKTOR_HOME/resources/build/configurations/make-project.sbm \
	$TRAKTOR_HOME/resources/build/ExternLinux.xms

$SOLUTIONBUILDER \
	-f=make2 \
	-make-solution-template=$TRAKTOR_HOME/resources/build/configurations/make-solution.sbm \
	-make-project-template=$TRAKTOR_HOME/resources/build/configurations/make-project.sbm \
	$TRAKTOR_HOME/resources/build/TraktorLinux.xms

