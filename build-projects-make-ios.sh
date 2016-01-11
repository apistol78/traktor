#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Generator configuration.
set MAKE_DIALECT=make
set MAKE_TOOLCHAIN=clang
set MAKE_OS=osx
set MAKE_INCLUDE=$(TRAKTOR_HOME)/bin/make-config-ios.inc
set MAKE_OBJECT_FILE=%%s.o
set MAKE_STATIC_LIBRARY_FILE=lib%%s.a
set MAKE_SHARED_LIBRARY_FILE=lib%%s.dylib
set MAKE_EXECUTABLE_FILE=%%s

# Build solution files.
$TRAKTOR_HOME/bin/osx/SolutionBuilder \
	-f=make2 \
	-make-solution-template=$TRAKTOR_HOME/bin/make-solution.sbm \
	-make-project-template=$TRAKTOR_HOME/bin/make-project.sbm \
	$TRAKTOR_HOME/TraktorIOS.xms
