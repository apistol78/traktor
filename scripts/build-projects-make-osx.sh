#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Generator configuration.
export MAKE_DIALECT=make
export MAKE_TOOLCHAIN=clang
export MAKE_OS=osx
export MAKE_INCLUDE="\$(TRAKTOR_HOME)/resources/build/configurations/make-config-osx.inc"
export MAKE_OBJECT_FILE="%s.o"
export MAKE_STATIC_LIBRARY_FILE="lib%s.a"
export MAKE_SHARED_LIBRARY_FILE="lib%s.dylib"
export MAKE_EXECUTABLE_FILE="%s"

if [ "$AGGREGATE_OUTPUT_PATH" == "" ]; then
	export AGGREGATE_OUTPUT_PATH="\$(TRAKTOR_HOME)/bin/latest/osx"
fi

# Build solution files.
$TRAKTOR_HOME/bin/osx/SolutionBuilder \
	-f=make2 \
	-make-solution-template=$TRAKTOR_HOME/resources/build/configurations/make-solution.sbm \
	-make-project-template=$TRAKTOR_HOME/resources/build/configurations/make-project.sbm \
	$TRAKTOR_HOME/resources/build/TraktorOSX.xms