#!/bin/bash

# Get normalized path to this script, excluding file name.
TRAKTOR_HOME="`dirname \"$BASH_SOURCE\"`/.."
TRAKTOR_HOME="`(cd \"$TRAKTOR_HOME\" && pwd)`"
export TRAKTOR_HOME

# Paths to 3rd party dependencies.
export STEAMWORKS_SDK=$TRAKTOR_HOME/3rdp/steamworks-sdk-142
export BULLET_SDK=$TRAKTOR_HOME/3rdp/bullet3-master-170526
export FBX_SDK=$TRAKTOR_HOME/3rdp/FbxSdk/2016.1.2
export GLSL_OPTIMIZER=$TRAKTOR_HOME/3rdp/glsl-optimizer-master-20141020
export SQLITE_SDK=$TRAKTOR_HOME/3rdp/sqlite-3.8.7.2
export WXWIDGETS_SDK=$TRAKTOR_HOME/3rdp/wxWidgets-3.0.2
export LUA_SDK=$TRAKTOR_HOME/3rdp/lua-5.3.2
export GLEW_SDK=$TRAKTOR_HOME/3rdp/glew-1.13.0
export EXPAT_SDK=$TRAKTOR_HOME/3rdp/libexpat-R_2_2_5/expat

# Export name of solution builder binary.
if [[ `uname -s` == Linux* ]]; then
	export SOLUTIONBUILDER="$TRAKTOR_HOME/bin/linux/releasestatic/Traktor.SolutionBuilder.App"
elif [[ `uname -s` == Darwin* ]]; then
	export SOLUTIONBUILDER="$TRAKTOR_HOME/bin/osx/releasestatic/Traktor.SolutionBuilder.App"
fi
