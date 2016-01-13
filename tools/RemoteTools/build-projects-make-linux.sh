#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../../config.sh"

# Generator configuration.
export MAKE_DIALECT=make
export MAKE_TOOLCHAIN=gcc
export MAKE_OS=linux
export MAKE_INCLUDE="\$(TRAKTOR_HOME)/bin/make-config-linux.inc"
export MAKE_OBJECT_FILE="%s.o"
export MAKE_STATIC_LIBRARY_FILE="lib%s.a"
export MAKE_SHARED_LIBRARY_FILE="lib%s.so"
export MAKE_EXECUTABLE_FILE="%s"

# Build solution files.
$TRAKTOR_HOME/bin/linux/SolutionBuilder \
	-f=make2 \
	-make-solution-template=$TRAKTOR_HOME/bin/make-solution.sbm \
	-make-project-template=$TRAKTOR_HOME/bin/make-project.sbm \
	RemoteToolsLinux.xms
