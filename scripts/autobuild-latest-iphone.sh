#!/bin/sh

source ../config.sh

# Build Traktor
#---------------
pushd $TRAKTOR_HOME
/bin/sh build-projects-xcode-iphone.sh

pushd build/iphone-static
xcodebuild -alltargets -configuration Release -sdk iphonesimulator
xcodebuild -alltargets -configuration Release -sdk iphoneos
popd

popd

# Put built binaries into place
#-------------------------------
/bin/sh copy-latest-iphone.sh

