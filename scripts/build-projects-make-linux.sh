#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Generator configuration.
export MAKE_OS=linux
export MAKE_DIALECT=make
export MAKE_FORMAT=$TRAKTOR_HOME/resources/build/configurations/make-format.sbm
export MAKE_INCLUDE="\$(TRAKTOR_HOME)/resources/build/configurations/make-config-linux.inc"

# Optional parameters.
export MAKE_TOOLCHAIN=gcc
export MAKE_OBJECT_FILE="%s.o"
export MAKE_STATIC_LIBRARY_FILE="lib%s.a"
export MAKE_SHARED_LIBRARY_FILE="lib%s.so"
export MAKE_EXECUTABLE_FILE="%s"
export MAKE_VERBOSE=false

# Steam redistributable aggregate.
if [ "`uname -m`" == "x86_64" ]; then
	export STEAMWORKS_SDK_BIN="$STEAMWORKS_SDK/redistributable_bin/linux64"
else
	export STEAMWORKS_SDK_BIN="$STEAMWORKS_SDK/redistributable_bin/linux32"
fi

# Create wayland XDG files.
WAYLAND_FLAGS=$(pkg-config wayland-client --cflags --libs)
WAYLAND_PROTOCOLS_DIR=$(pkg-config wayland-protocols --variable=pkgdatadir)
WAYLAND_SCANNER=$(pkg-config --variable=wayland_scanner wayland-scanner)

XDG_SHELL_PROTOCOL=$WAYLAND_PROTOCOLS_DIR/stable/xdg-shell/xdg-shell.xml
XDG_OUTPUT_PATH="`dirname \"$BASH_SOURCE\"`/../code/Ui/WL/xdg"

mkdir $XDG_OUTPUT_PATH
$WAYLAND_SCANNER client-header $XDG_SHELL_PROTOCOL $XDG_OUTPUT_PATH/xdg-shell-client-protocol.h
$WAYLAND_SCANNER private-code $XDG_SHELL_PROTOCOL $XDG_OUTPUT_PATH/xdg-shell-protocol.c

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

