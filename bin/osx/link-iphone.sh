#!/bin/sh
set -e

ARG_BUNDLE=${1}
ARG_EXECUTABLE=${2}
ARG_MODULES=${*:3}

LD=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++
SDK_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator7.0.sdk
FORCE="-Xlinker -force_load -Xlinker "

# Mandatory modules.
LD_MODULES="
	$FORCE libTraktor.Amalgam.App.a
	$FORCE libTraktor.Ai.a
	$FORCE libTraktor.Animation.a
	$FORCE libTraktor.Core.a
	$FORCE libTraktor.World.a
	$FORCE libTraktor.Render.a
	$FORCE libTraktor.Resource.a
	$FORCE libTraktor.Compress.a
	$FORCE libTraktor.Database.a
	$FORCE libTraktor.Xml.a
	$FORCE libTraktor.Mesh.a
	$FORCE libTraktor.Physics.a
	$FORCE libTraktor.Heightfield.a
	$FORCE libTraktor.Net.a
	$FORCE libTraktor.Drawing.a
	$FORCE libTraktor.Flash.a
	$FORCE libTraktor.Html.a
	$FORCE libTraktor.Sound.a
	$FORCE libTraktor.I18N.a
	$FORCE libTraktor.Input.a
	$FORCE libTraktor.Scene.a
	$FORCE libTraktor.Script.a
	$FORCE libTraktor.Spray.a
	$FORCE libTraktor.Terrain.a
	$FORCE libTraktor.Theater.a
	$FORCE libTraktor.Weather.a
	$FORCE libTraktor.Video.a
	$FORCE libTraktor.Online.a
	$FORCE libTraktor.Amalgam.a
	libExtern.bullet.a
	libExtern.expat-2.1.0.a
	libExtern.jpeg-6b.a
	libExtern.libflac-1.2.1.a
	libExtern.libogg-1.3.0.a
	libExtern.libvorbis-1.3.3.a
	libExtern.lpng128.a
	libExtern.lua-5.1.4.a
	libExtern.zlib-1.2.3.a
	libExtern.liblzf-3.6.a
	libExtern.minilzo-2.05.a
	libExtern.mpg123-1.13.2.a
	libExtern.sqlite-3.7.6.3.a
	libExtern.libtheora-1.1.1.a
	libExtern.stb_vorbis.a
	libExtern.detour.a
	libExtern.recast.a
"

# Add dynamic modules from command line.
for MODULE in $ARG_MODULES
do
	LD_MODULES="$LD_MODULES $FORCE lib$MODULE.a"
done

# Mandatory frameworks
LD_FRAMEWORKS="
	-framework Foundation
	-framework OpenAL
	-framework OpenGLES
	-framework QuartzCore
	-framework UIKit
"

# Invoke linker to build final executable.
rm -Rf $ARG_EXECUTABLE
$LD \
	-arch i386 \
	-isysroot $SDK_SYSROOT \
	-miphoneos-version-min=7.0 \
	-Xlinker -objc_abi_version -Xlinker 2 \
	-Xlinker -no_implicit_dylibs \
	-Xlinker -u -Xlinker _main \
	-D__IPHONE_OS_VERSION_MIN_REQUIRED=70000 \
	$LD_FRAMEWORKS \
	$LD_MODULES \
	-o $ARG_EXECUTABLE

# Build application bundle.
if [ ! -d "$ARG_BUNDLE.app" ]; then
	mkdir "$ARG_BUNDLE.app"
fi
rm -Rf "$ARG_BUNDLE.app"/*
cp $ARG_EXECUTABLE "$ARG_BUNDLE.app"
cp Info.plist "$ARG_BUNDLE.app"
plutil -convert binary1 "$ARG_BUNDLE.app"/Info.plist
rm $ARG_EXECUTABLE

echo "iPhone executable built successfully"
