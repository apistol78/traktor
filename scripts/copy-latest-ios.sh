#!/bin/sh

source ../config.sh

CONFIG=$1
if [ -z $CONFIG ] ; then
	CONFIG="both"
fi

if [ $CONFIG == "both" ] || [ $CONFIG == "release" ] ; then

	# Copy archives.
	cp -R -f $TRAKTOR_HOME/build/ios/releasestatic/* bin/latest/ios/releasestatic/

	# Copy 3rd-party frameworks.
	cp -R -f $TRAKTOR_HOME/3rdp/Everyplay/Everyplay.bundle bin/latest/ios/releasestatic/
	cp -R -f $TRAKTOR_HOME/3rdp/Everyplay/Everyplay.framework bin/latest/ios/releasestatic/

	popd

fi

if [ $CONFIG == "both" ] || [ $CONFIG == "debug" ] ; then

	# Copy archives.
	cp -R -f $TRAKTOR_HOME/build/ios/releasestatic/* bin/latest/ios/debugstatic/

	# Copy 3rd-party frameworks.
	cp -R -f $TRAKTOR_HOME/3rdp/Everyplay/Everyplay.bundle bin/latest/ios/debugstatic/
	cp -R -f $TRAKTOR_HOME/3rdp/Everyplay/Everyplay.framework bin/latest/ios/debugstatic/

	popd

fi
