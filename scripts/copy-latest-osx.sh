#!/bin/sh

source ../config.sh

# RELEASE

# Copy products into bin directory.
pushd $TRAKTOR_HOME

mkdir -p bin/latest/osx/releaseshared
cd bin/latest/osx/releaseshared

TRAKTOR_BUILD=$TRAKTOR_HOME/build/osx/releaseshared

# Copy binaries.
cp -f $TRAKTOR_BUILD/*.dylib .
cp -f $TRAKTOR_BUILD/Traktor.Amalgam.App .
cp -f $TRAKTOR_BUILD/Traktor.Editor.App .
cp -f $TRAKTOR_BUILD/Traktor.Pipeline.App .
cp -f $TRAKTOR_BUILD/Traktor.Database.Migrate.App .
cp -f $TRAKTOR_BUILD/Traktor.Database.Remote.Server.App .

# Copy Steam requirements.
cp -f $STEAMWORKS_SDK/redistributable_bin/osx32/libsteam_api.dylib .

# Copy FBX dependencies.
cp -f $FBX_SDK/lib/clang/ub/release/libfbxsdk.dylib .

# Assemble Editor bundle.
mkdir -p TraktorEditor.app/Contents/MacOS
mkdir -p TraktorEditor.app/Contents/Resources

cp -f *.dylib TraktorEditor.app/Contents/MacOS
cp -f Traktor.Editor.App TraktorEditor.app/Contents/MacOS
cp -f $TRAKTOR_HOME/res/osx/Info.plist TraktorEditor.app/Contents
cp -f $TRAKTOR_HOME/Traktor.Editor.config TraktorEditor.app/Contents/Resources
cp -f $TRAKTOR_HOME/Traktor.Editor.osx.config TraktorEditor.app/Contents/Resources

popd

# DEBUG

# Copy products into bin directory.
pushd $TRAKTOR_HOME

mkdir -p bin/latest/osx/debugshared
cd bin/latest/osx/debugshared

TRAKTOR_BUILD=$TRAKTOR_HOME/build/osx/debugshared

# Copy binaries.
cp -f $TRAKTOR_BUILD/*.dylib .
cp -f $TRAKTOR_BUILD/Traktor.Amalgam.App_d .
cp -f $TRAKTOR_BUILD/Traktor.Editor.App_d .
cp -f $TRAKTOR_BUILD/Traktor.Pipeline.App_d .
cp -f $TRAKTOR_BUILD/Traktor.Database.Migrate.App_d .
cp -f $TRAKTOR_BUILD/Traktor.Database.Remote.Server.App_d .

# Copy Steam requirements.
cp -f $STEAMWORKS_SDK/redistributable_bin/osx32/libsteam_api.dylib .

# Copy FBX dependencies.
cp -f $FBX_SDK/lib/clang/ub/release/libfbxsdk.dylib .

# Assemble Editor bundle.
mkdir -p TraktorEditor.app/Contents/MacOS
mkdir -p TraktorEditor.app/Contents/Resources

cp -f *.dylib TraktorEditor.app/Contents/MacOS
cp -f Traktor.Editor.App_d TraktorEditor.app/Contents/MacOS
cp -f $TRAKTOR_HOME/res/osx/Info_d.plist TraktorEditor.app/Contents/Info.plist
cp -f $TRAKTOR_HOME/Traktor.Editor.config TraktorEditor.app/Contents/Resources
cp -f $TRAKTOR_HOME/Traktor.Editor.osx.config TraktorEditor.app/Contents/Resources

popd
