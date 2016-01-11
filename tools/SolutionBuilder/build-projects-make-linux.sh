#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../../config.sh"

# Generator configuration.
set MAKE_DIALECT=make
set MAKE_TOOLCHAIN=gcc
set MAKE_OS=linux
set MAKE_INCLUDE=$(TRAKTOR_HOME)/bin/make-config-osx.inc
set MAKE_OBJECT_FILE=%%s.o
set MAKE_STATIC_LIBRARY_FILE=lib%%s.a
set MAKE_SHARED_LIBRARY_FILE=lib%%s.so
set MAKE_EXECUTABLE_FILE=%%s

# Build solution files.
$TRAKTOR_HOME/bin/linux/SolutionBuilder \
	-f=make2 \
	-make-solution-template=$TRAKTOR_HOME/bin/make-solution.sbm \
	-make-project-template=$TRAKTOR_HOME/bin/make-project.sbm \
	SolutionBuilderLinux.xms
