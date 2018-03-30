# Configuration file

# Toolchain and SDK configuration.
TOOLCHAIN_BIN=$(ANDROID_NDK_ROOT)/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin
NDK_INCLUDES=\
	-I$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/include\
	-I$(ANDROID_NDK_ROOT)/sources/android/support/include\
	-I$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++abi/include\
	-I$(ANDROID_NDK_ROOT)/sysroot/usr/include/i686-linux-android\
	-I$(ANDROID_NDK_ROOT)/sysroot/usr/include

# Build tools.
C=$(TOOLCHAIN_BIN)/arm-linux-androideabi-g++

C_FLAGS_COMMON=\
	-fno-strict-aliasing\
	-fno-common\
	-pthread\
	-MMD\
	-marm\
	-march=armv7-a\
	-mtune=xscale\
	-fno-strict-aliasing\
	-funswitch-loops\
	-finline-limit=100\
	-fomit-frame-pointer\
	-fno-exceptions\
	-fstack-protector\
	-fno-rtti\
	-fno-short-enums\
	-fPIC\
	-x c++\
	-Wno-psabi\
	-Wno-attributes\
	-D __ANDROID__\
	$(NDK_INCLUDES)

C_FLAGS_DEBUG_SHARED=$(C_FLAGS_COMMON) -g -D _UNICODE -D UNICODE
C_FLAGS_RELEASE_SHARED=$(C_FLAGS_COMMON) -O2 -D _UNICODE -D UNICODE -D NDEBUG
C_FLAGS_DEBUG_STATIC=$(C_FLAGS_COMMON) -g -D _UNICODE -D UNICODE
C_FLAGS_RELEASE_STATIC=$(C_FLAGS_COMMON) -O2 -D _UNICODE -D UNICODE -D NDEBUG
C_FLAGS_DEBUG_EXECUTABLE=$(C_FLAGS_COMMON) -g -D _UNICODE -D UNICODE
C_FLAGS_RELEASE_EXECUTABLE=$(C_FLAGS_COMMON) -O2 -D _UNICODE -D UNICODE -D NDEBUG
C_FLAG_OUTPUT=-o 


CC=$(TOOLCHAIN_BIN)/arm-linux-androideabi-g++

CC_FLAGS_COMMON=\
	-std=c++11\
	-fno-strict-aliasing\
	-fno-common\
	-pthread\
	-MMD\
	-marm\
	-march=armv7-a\
	-mtune=xscale\
	-fno-strict-aliasing\
	-funswitch-loops\
	-finline-limit=100\
	-fomit-frame-pointer\
	-fno-exceptions\
	-fstack-protector\
	-fno-rtti\
	-fno-short-enums\
	-fPIC\
	-x c++\
	-Wno-psabi\
	-Wno-attributes\
	-D __ANDROID__\
	$(NDK_INCLUDES)

CC_FLAGS_DEBUG_SHARED=$(CC_FLAGS_COMMON) -g -D _UNICODE -D UNICODE
CC_FLAGS_RELEASE_SHARED=$(CC_FLAGS_COMMON) -O2 -D _UNICODE -D UNICODE -D NDEBUG
CC_FLAGS_DEBUG_STATIC=$(CC_FLAGS_COMMON) -g -D _UNICODE -D UNICODE
CC_FLAGS_RELEASE_STATIC=$(CC_FLAGS_COMMON) -O2 -D _UNICODE -D UNICODE -D NDEBUG
CC_FLAGS_DEBUG_EXECUTABLE=$(CC_FLAGS_COMMON) -g -D _UNICODE -D UNICODE
CC_FLAGS_RELEASE_EXECUTABLE=$(CC_FLAGS_COMMON) -O2 -D _UNICODE -D UNICODE -D NDEBUG
CC_FLAG_OUTPUT=-o 

AR=$(TOOLCHAIN_BIN)/arm-linux-androideabi-ar
AR_FLAGS_COMMON=-r -s
AR_FLAGS_DEBUG_STATIC=$(AR_FLAGS_COMMON)
AR_FLAGS_RELEASE_STATIC=$(AR_FLAGS_COMMON)
AR_FLAG_OUTPUT=

LINK=$(TOOLCHAIN_BIN)/arm-linux-androideabi-g++
LINK_LIBS=
LINK_FLAG_OUTPUT=-o

ADDITIONAL_INCLUDE_PATHS=
ADDITIONAL_LIBRARY_PATHS=

BINARY_INCLUDE=$(TRAKTOR_HOME)/bin/linux/BinaryInclude
