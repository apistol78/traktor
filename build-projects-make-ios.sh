#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Build solution files.
$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=make -make-dialect=gnu -make-platform=ios -make-configuration=$TRAKTOR_HOME/bin/make-config-ios-armv7.inc -make-root-suffix=-armv7 TraktorIOS.xms
#$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=make -make-dialect=gnu -make-platform=ios -make-configuration=$TRAKTOR_HOME/bin/make-config-ios-armv7s.inc -make-root-suffix=-armv7s TraktorIOS.xms
#$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=make -make-dialect=gnu -make-platform=ios -make-configuration=$TRAKTOR_HOME/bin/make-config-ios-arm64.inc -make-root-suffix=-arm64 TraktorIOS.xms
$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=make -make-dialect=gnu -make-platform=ios -make-configuration=$TRAKTOR_HOME/bin/make-config-ios-i386.inc -make-root-suffix=-i386 TraktorIOS.xms
#$TRAKTOR_HOME/bin/osx/SolutionBuilder -f=make -make-dialect=gnu -make-platform=ios -make-configuration=$TRAKTOR_HOME/bin/make-config-ios-x86_64.inc -make-root-suffix=-x86_64 TraktorIOS.xms
