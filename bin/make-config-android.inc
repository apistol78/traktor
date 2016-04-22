# Configuration file

# Toolchain and SDK configuration.
TOOLCHAIN_BIN=$(ANDROID_NDK_ROOT)/toolchains/arm-linux-androideabi-4.8/prebuilt/darwin-x86_64/bin

# Build tools.
C=$(TOOLCHAIN_BIN)/arm-linux-androideabi-g++
C_FLAGS_COMMON=-fno-strict-aliasing -fno-common -pthread -MMD -marm -march=armv7-a -mtune=xscale -fno-strict-aliasing -funswitch-loops -finline-limit=100 -fomit-frame-pointer -fno-exceptions -fstack-protector -fno-rtti -fno-short-enums -fPIC -x c++ -Wno-psabi -D __ANDROID__ -I$(ANDROID_NDK_ROOT)/platforms/android-19/arch-arm/usr/include -I$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/include -I$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a/include
C_FLAGS_DEBUG_SHARED=$(C_FLAGS_COMMON) -g -D _UNICODE -D UNICODE
C_FLAGS_RELEASE_SHARED=$(C_FLAGS_COMMON) -O2 -D _UNICODE -D UNICODE -D NDEBUG
C_FLAGS_DEBUG_STATIC=$(C_FLAGS_COMMON) -g -D _UNICODE -D UNICODE
C_FLAGS_RELEASE_STATIC=$(C_FLAGS_COMMON) -O2 -D _UNICODE -D UNICODE -D NDEBUG
C_FLAGS_DEBUG_EXECUTABLE=$(C_FLAGS_COMMON) -g -D _UNICODE -D UNICODE
C_FLAGS_RELEASE_EXECUTABLE=$(C_FLAGS_COMMON) -O2 -D _UNICODE -D UNICODE -D NDEBUG

CC=$(TOOLCHAIN_BIN)/arm-linux-androideabi-g++
CC_FLAGS_COMMON=-fno-strict-aliasing -fno-common -pthread -MMD -marm -march=armv7-a -mtune=xscale -fno-strict-aliasing -funswitch-loops -finline-limit=100 -fomit-frame-pointer -fno-exceptions -fstack-protector -fno-rtti -fno-short-enums -fPIC -x c++ -Wno-psabi -D __ANDROID__ -I$(ANDROID_NDK_ROOT)/platforms/android-19/arch-arm/usr/include -I$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/include -I$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a/include
CC_FLAGS_DEBUG_SHARED=$(CC_FLAGS_COMMON) -g -D _UNICODE -D UNICODE
CC_FLAGS_RELEASE_SHARED=$(CC_FLAGS_COMMON) -O2 -D _UNICODE -D UNICODE -D NDEBUG
CC_FLAGS_DEBUG_STATIC=$(CC_FLAGS_COMMON) -g -D _UNICODE -D UNICODE
CC_FLAGS_RELEASE_STATIC=$(CC_FLAGS_COMMON) -O2 -D _UNICODE -D UNICODE -D NDEBUG
CC_FLAGS_DEBUG_EXECUTABLE=$(CC_FLAGS_COMMON) -g -D _UNICODE -D UNICODE
CC_FLAGS_RELEASE_EXECUTABLE=$(CC_FLAGS_COMMON) -O2 -D _UNICODE -D UNICODE -D NDEBUG

AR=$(TOOLCHAIN_BIN)/arm-linux-androideabi-ar
AR_FLAGS_COMMON=-r -s
AR_FLAGS_DEBUG_STATIC=$(AR_FLAGS_COMMON)
AR_FLAGS_RELEASE_STATIC=$(AR_FLAGS_COMMON)

LINK=$(TOOLCHAIN_BIN)/arm-linux-androideabi-g++
LINK_LIBS=

ADDITIONAL_INCLUDE_PATHS=
ADDITIONAL_LIBRARY_PATHS=

BINARY_INCLUDE=BinaryInclude
