#!/bin/sh

NAME="$1"
IPA="$1.ipa"
BUNDLE="$1.app"

# Remove existing file.
rm -Rf "image/$NAME" 2> /dev/null | true
rm -Rf $IPA 2> /dev/null | true

# Create image.
mkdir -p "image/$NAME/Payload"
cp -Rf $BUNDLE "image/$NAME/Payload"

# Create ipa file.
pushd image

# Sign bundle.
/usr/bin/codesign -f -s \
	"iPhone Distribution: Doctor Entertainment AB" \
	--resource-rules "$NAME/Payload/$BUNDLE/ResourceRules.plist" \
	"$NAME/Payload/$BUNDLE"

# Package
cd $NAME
zip -qr "../../$IPA" *

popd
