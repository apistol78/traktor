#!/bin/bash

# Get normalized path to this script, excluding file name.
TRAKTOR_HOME="`dirname \"$BASH_SOURCE\"`"
TRAKTOR_HOME="`(cd \"$TRAKTOR_HOME\" && pwd)`"

export TRAKTOR_HOME
export STEAMWORKS_SDK=$TRAKTOR_HOME/3rdp/steamworks-sdk-123a
export BULLET_SDK=$TRAKTOR_HOME/3rdp/bullet-2.81-rev2643