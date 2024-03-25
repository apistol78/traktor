#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../config.sh"

mkdir -p "$TRAKTOR_HOME/build/archive/traktor"
pushd "$TRAKTOR_HOME/build/archive/traktor"

mkdir -p ./bin/latest/linux
mkdir -p ./resources/runtime
mkdir -p ./scripts/runtime
mkdir -p ./data/Assets
mkdir -p ./data/Source

cp -r $TRAKTOR_HOME/bin/latest/linux/releaseshared/ ./bin/latest/linux/
cp -r $TRAKTOR_HOME/data/Assets/ ./data/
cp -r $TRAKTOR_HOME/data/Source/ ./data/
cp -r $TRAKTOR_HOME/resources/runtime/ ./resources/
cp -r $TRAKTOR_HOME/scripts/runtime/ ./scripts/
cp $TRAKTOR_HOME/LICENSE.txt .
cp $TRAKTOR_HOME/README.md .

popd