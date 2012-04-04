#!/bin/sh

source ../config.sh

# Build Traktor
#---------------
pushd $TRAKTOR_HOME
/bin/sh build-projects-xcode-osx.sh

pushd build/macosx-shared
xcodebuild -alltargets -configuration Release
popd

pushd build/macosx-static
xcodebuild -alltargets -configuration Release
popd

popd

# Put built binaries into place
#-------------------------------
/bin/sh copy-latest-osx.sh

