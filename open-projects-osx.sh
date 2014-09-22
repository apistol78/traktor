#!/bin/sh

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Open Xcode.
open "build/osx-shared/Traktor OSX.xcodeproj"
