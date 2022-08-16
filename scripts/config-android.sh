#!/bin/bash

# Android SDK
if [[ `uname -s` == Linux* ]]; then
	export ANDROID_HOME=$TRAKTOR_HOME/3rdp_/android-sdk-linux
elif [[ `uname -s` == Darwin* ]]; then
	export ANDROID_HOME=$TRAKTOR_HOME/3rdp_/android-sdk-macOS
fi

# Android NDK
export ANDROID_NDK_ROOT=$ANDROID_HOME/ndk-bundle
