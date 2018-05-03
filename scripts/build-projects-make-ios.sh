#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Mandatory parameters.
export MAKE_OS=osx
export MAKE_DIALECT=make
export MAKE_FORMAT=$TRAKTOR_HOME/resources/build/configurations/make-format.sbm
export MAKE_INCLUDE="\$(TRAKTOR_HOME)/resources/build/configurations/make-config-ios.inc"
export MAKE_ARCHITECTURES="armv7,armv7s,arm64,i386,x86_64"

# Used by default format.
export MAKE_TOOLCHAIN=clang
export MAKE_OBJECT_FILE=%s.o
export MAKE_STATIC_LIBRARY_FILE=lib%s.a
export MAKE_SHARED_LIBRARY_FILE=lib%s.dylib
export MAKE_EXECUTABLE_FILE=%s

$SOLUTIONBUILDER \
	-f=make2 \
	-make-solution-template=$TRAKTOR_HOME/resources/build/configurations/make-solution.sbm \
	-make-project-template=$TRAKTOR_HOME/resources/build/configurations/make-project.sbm \
	$TRAKTOR_HOME/resources/build/ExternIOS.xms

$SOLUTIONBUILDER \
	-f=make2 \
	-make-solution-template=$TRAKTOR_HOME/resources/build/configurations/make-solution.sbm \
	-make-project-template=$TRAKTOR_HOME/resources/build/configurations/make-project.sbm \
	$TRAKTOR_HOME/resources/build/TraktorIOS.xms
