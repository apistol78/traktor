# Configuration file

# Toolchain and SDK configuration.
SYSROOT=$(RPI_SYSROOT)

# Build tools.
C=clang
C_FLAGS_COMMON=\
	--target=arm-linux-gnueabihf\
	--sysroot=$(SYSROOT)\
    -fno-common\
    -D _UNICODE\
    -D UNICODE\
    -D __RPI__\
    -fPIC\
    -fvisibility=hidden

C_FLAGS_DEBUG_SHARED=$(C_FLAGS_COMMON) -O1 -g -fno-omit-frame-pointer 
C_FLAGS_RELEASE_SHARED=$(C_FLAGS_COMMON) -O2 -g1
C_FLAGS_DEBUG_STATIC=$(C_FLAGS_COMMON) -O1 -g -fno-omit-frame-pointer 
C_FLAGS_RELEASE_STATIC=$(C_FLAGS_COMMON) -O2 -g1
C_FLAGS_DEBUG_EXECUTABLE=$(C_FLAGS_COMMON) -O1 -g -fno-omit-frame-pointer 
C_FLAGS_RELEASE_EXECUTABLE=$(C_FLAGS_COMMON) -O2 -g1
C_FLAG_OUTPUT=-o 

CC=clang++
CC_FLAGS_COMMON=\
	--target=arm-linux-gnueabihf\
	--sysroot=$(SYSROOT)\
    -fno-common\
    -std=c++20\
    -D _UNICODE\
    -D UNICODE\
    -D __RPI__\
    -fPIC\
    -fvisibility=hidden

CC_FLAGS_DEBUG_SHARED=$(CC_FLAGS_COMMON) -O1 -g -fno-omit-frame-pointer 
CC_FLAGS_RELEASE_SHARED=$(CC_FLAGS_COMMON) -O2 -g1
CC_FLAGS_DEBUG_STATIC=$(CC_FLAGS_COMMON) -O1 -g -fno-omit-frame-pointer 
CC_FLAGS_RELEASE_STATIC=$(CC_FLAGS_COMMON) -O2 -g1
CC_FLAGS_DEBUG_EXECUTABLE=$(CC_FLAGS_COMMON) -O1 -g -fno-omit-frame-pointer 
CC_FLAGS_RELEASE_EXECUTABLE=$(CC_FLAGS_COMMON) -O2 -g1
CC_FLAG_OUTPUT=-o 

AR=ar
AR_FLAGS_COMMON=-r -s
AR_FLAGS_DEBUG_STATIC=$(AR_FLAGS_COMMON)
AR_FLAGS_RELEASE_STATIC=$(AR_FLAGS_COMMON)
AR_FLAG_OUTPUT=

LINK=clang++
LINK_FLAGS_COMMON=\
    -fuse-ld=lld\
    --target=arm-linux-gnueabihf\
    --sysroot=$(SYSROOT)\
    -lpthread\
    -ldl\
    -Wl,-rpath=\$$ORIGIN\
    -Wl,-gc-sections

LINK_FLAGS_DEBUG_SHARED=$(LINK_FLAGS_COMMON) -g -shared 
LINK_FLAGS_RELEASE_SHARED=$(LINK_FLAGS_COMMON) -g1 -shared 
LINK_FLAGS_DEBUG_STATIC=$(LINK_FLAGS_COMMON) -g
LINK_FLAGS_RELEASE_STATIC=$(LINK_FLAGS_COMMON) -g1
LINK_FLAGS_DEBUG_EXECUTABLE=$(LINK_FLAGS_COMMON) -g 
LINK_FLAGS_RELEASE_EXECUTABLE=$(LINK_FLAGS_COMMON) -g1
LINK_FLAG_OUTPUT=-o 
LINK_LIBS=

ADDITIONAL_INCLUDE_PATHS=
ADDITIONAL_LIBRARY_PATHS=

BINARY_INCLUDE=$(RUN) $(TRAKTOR_HOME)/scripts/binaryinclude/BinaryInclude.run
