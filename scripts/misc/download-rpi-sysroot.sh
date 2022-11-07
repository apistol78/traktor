#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/config.sh"

# Local sysroot path.
export SYSROOT=$TRAKTOR_HOME/3rdp-restricted/raspberry-sysroot

echo "Cleaning local sysroot \"$SYSROOT\"..."

rm -rf $SYSROOT
mkdir $SYSROOT
mkdir $SYSROOT/usr
mkdir $SYSROOT/usr/include
mkdir $SYSROOT/usr/local
mkdir $SYSROOT/usr/opt
mkdir $SYSROOT/usr/opt/vc

echo "Downloading sysroot into \"$SYSROOT\"..."

rsync -r -a -v -z -e ssh pi@pn-borvo-rpi:/lib/ $SYSROOT/lib/
rsync -r -a -v -z -e ssh pi@pn-borvo-rpi:/usr/include/ $SYSROOT/usr/include/
rsync -r -a -v -z -e ssh pi@pn-borvo-rpi:/usr/lib/ $SYSROOT/usr/lib/
rsync -r -a -v -z -e ssh pi@pn-borvo-rpi:/opt/vc/ $SYSROOT/opt/vc/

echo "Modifying symbolic links..."

$TRAKTOR_HOME/scripts/sysroot-relativelinks.py $SYSROOT

echo "Done"

