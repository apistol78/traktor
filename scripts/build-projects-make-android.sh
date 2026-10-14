#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"
. "`dirname \"$BASH_SOURCE\"`/config-android.sh"

# Determine platform from some simple heuristic.
if [[ `uname -s` == Linux* ]]; then
	if [[ `uname -m` == arm* ]]; then
		export TRAKTOR_PLATFORM=rpi
	else
		export TRAKTOR_PLATFORM=linux
	fi
elif [[ `uname -s` == Darwin* ]]; then
	export TRAKTOR_PLATFORM=macos
fi

export SOLUTIONBUILDER="$TRAKTOR_HOME/bin/${TRAKTOR_PLATFORM/macos/osx}/releasestatic/Traktor.SolutionBuilder.App"

# Mandatory parameters.
export MAKE_DIALECT=make
export MAKE_FORMAT=$TRAKTOR_HOME/resources/build/configurations/make-format.sbm
if [[ `uname -s` == Linux* ]]; then
	export MAKE_OS=linux
	export MAKE_INCLUDE="\$(TRAKTOR_HOME)/resources/build/configurations/make-config-android-linux.inc"
elif [[ `uname -s` == Darwin* ]]; then
	export MAKE_OS=osx
	export MAKE_INCLUDE="\$(TRAKTOR_HOME)/resources/build/configurations/make-config-android-darwin.inc"
fi

# Used by default format.
export MAKE_TOOLCHAIN=clang
export MAKE_OBJECT_FILE=%s.o
export MAKE_STATIC_LIBRARY_FILE=lib%s.a
export MAKE_SHARED_LIBRARY_FILE=lib%s.so
export MAKE_EXECUTABLE_FILE=%s

$SOLUTIONBUILDER \
	-f=make2 \
	-make-solution-template=$TRAKTOR_HOME/resources/build/configurations/make-solution.sbm \
	-make-project-template=$TRAKTOR_HOME/resources/build/configurations/make-project.sbm \
	$TRAKTOR_HOME/resources/build/ExternAndroid.xms

$SOLUTIONBUILDER \
	-f=make2 \
	-make-solution-template=$TRAKTOR_HOME/resources/build/configurations/make-solution.sbm \
	-make-project-template=$TRAKTOR_HOME/resources/build/configurations/make-project.sbm \
	$TRAKTOR_HOME/resources/build/TraktorAndroid.xms

