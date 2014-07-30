#!/bin/sh

source ../config.sh

# All build outputs
TRAKTOR_BUILD_i386=$TRAKTOR_HOME/build/ios-i386/releasestatic
#TRAKTOR_BUILD_x86_64=$TRAKTOR_HOME/build/ios-x86_64/releasestatic
TRAKTOR_BUILD_armv7=$TRAKTOR_HOME/build/ios-armv7/releasestatic
#TRAKTOR_BUILD_armv7s=$TRAKTOR_HOME/build/ios-armv7s/releasestatic
#TRAKTOR_BUILD_arm64=$TRAKTOR_HOME/build/ios-arm64/releasestatic

# Generate LIPO archive of all architectures.
pushd $TRAKTOR_HOME
mkdir -p bin/latest/ios/releasestatic

shopt -s nullglob

FILES="$TRAKTOR_BUILD_i386/*.a"
for FILE in $FILES
do
	FILE=$(basename $FILE)
	echo "Creating universal archive $FILE..."
#	lipo -create -output "bin/latest/ios/releasestatic/$FILE" -arch i386 "$TRAKTOR_BUILD_i386/$FILE" -arch x86_64 "$TRAKTOR_BUILD_x86_64/$FILE" -arch armv7 "$TRAKTOR_BUILD_armv7/$FILE" -arch armv7s "$TRAKTOR_BUILD_armv7s/$FILE" -arch arm64 "$TRAKTOR_BUILD_arm64/$FILE"
	lipo -create -output "bin/latest/ios/releasestatic/$FILE" -arch i386 "$TRAKTOR_BUILD_i386/$FILE" -arch armv7 "$TRAKTOR_BUILD_armv7/$FILE"
done

popd

# Generate LIPO archive of all architectures.
pushd $TRAKTOR_HOME
mkdir -p bin/latest/ios/debugstatic

shopt -s nullglob

FILES="$TRAKTOR_BUILD_i386/*.a"
for FILE in $FILES
do
	FILE=$(basename $FILE)
	echo "Creating universal archive $FILE..."
#	lipo -create -output "bin/latest/ios/releasestatic/$FILE" -arch i386 "$TRAKTOR_BUILD_i386/$FILE" -arch x86_64 "$TRAKTOR_BUILD_x86_64/$FILE" -arch armv7 "$TRAKTOR_BUILD_armv7/$FILE" -arch armv7s "$TRAKTOR_BUILD_armv7s/$FILE" -arch arm64 "$TRAKTOR_BUILD_arm64/$FILE"
	lipo -create -output "bin/latest/ios/debugstatic/$FILE" -arch i386 "$TRAKTOR_BUILD_i386/$FILE" -arch armv7 "$TRAKTOR_BUILD_armv7/$FILE"
done

popd