#!/bin/sh

source ../config.sh

# Copy products into bin directory.
pushd $TRAKTOR_HOME

mkdir -p bin/latest/osx/releaseshared
cd bin/latest/osx/releaseshared

TRAKTOR_BUILD=$TRAKTOR_HOME/build/osx/releaseshared

# Copy binaries.
cp -f $TRAKTOR_BUILD/*.dylib .
cp -f $TRAKTOR_BUILD/Traktor.Amalgam.App .
cp -f $TRAKTOR_BUILD/Traktor.Pipeline.App .
cp -f $TRAKTOR_BUILD/Traktor.Database.Migrate.App .
cp -f $TRAKTOR_BUILD/Traktor.Database.Remote.Server.App .

# Copy Steam requirements.
cp -f $STEAMWORKS_SDK/redistributable_bin/osx32/libsteam_api.dylib .

popd
