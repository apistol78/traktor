#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../config.sh"

OUTPUT=$TRAKTOR_HOME/build/distribution

rm -rf $OUTPUT

pushd $TRAKTOR_HOME

mkdir -p $OUTPUT/bin/latest
mkdir -p $OUTPUT/code
mkdir -p $OUTPUT/resources
mkdir -p $OUTPUT/scripts
mkdir -p $OUTPUT/data

# Binaries
cp -r bin/linux $OUTPUT/bin/
cp -r bin/osx $OUTPUT/bin/
cp -r bin/win32 $OUTPUT/bin/
cp -r bin/win64 $OUTPUT/bin/
cp -r bin/latest $OUTPUT/bin/

# Headers
rsync -av --include "*/" --include "*.h" --include "*.inl" --exclude "*" code/. $OUTPUT/code/.

# Resources
cp -r resources/build $OUTPUT/resources/
cp -r resources/icons $OUTPUT/resources/
cp -r resources/runtime $OUTPUT/resources/

# Scripts
cp -r scripts/binaryinclude $OUTPUT/scripts/
cp -r scripts/runtime $OUTPUT/scripts/

cp scripts/config.* $OUTPUT/scripts/
cp scripts/vsenv* $OUTPUT/scripts/
cp scripts/run* $OUTPUT/scripts/

# Data
cp -r data/Assets $OUTPUT/data/
cp -r data/Source $OUTPUT/data/

# Workspace
cp *.workspace $OUTPUT/

popd
