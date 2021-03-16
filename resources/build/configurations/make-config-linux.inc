# Configuration file

C=ccache gcc
C_FLAGS_COMMON=-fno-common -march=core2 -msse4.1 -D _UNICODE -D UNICODE -D __LINUX__ -fPIC -fvisibility=hidden
C_FLAGS_DEBUG_SHARED=$(C_FLAGS_COMMON) -O1 -g -fno-omit-frame-pointer -fsanitize=address
C_FLAGS_RELEASE_SHARED=$(C_FLAGS_COMMON) -O3 -g
C_FLAGS_DEBUG_STATIC=$(C_FLAGS_COMMON) -O1 -g -fno-omit-frame-pointer -fsanitize=address
C_FLAGS_RELEASE_STATIC=$(C_FLAGS_COMMON) -O3 -g
C_FLAGS_DEBUG_EXECUTABLE=$(C_FLAGS_COMMON) -O1 -g -fno-omit-frame-pointer -fsanitize=address
C_FLAGS_RELEASE_EXECUTABLE=$(C_FLAGS_COMMON) -O3 -g
C_FLAG_OUTPUT=-o 

CC=ccache gcc
CC_FLAGS_COMMON=-fno-common -std=c++14 -march=core2 -msse4.1 -D _UNICODE -D UNICODE -D __LINUX__ -fPIC -fvisibility=hidden
CC_FLAGS_DEBUG_SHARED=$(CC_FLAGS_COMMON) -O1 -g -fno-omit-frame-pointer -fsanitize=address
CC_FLAGS_RELEASE_SHARED=$(CC_FLAGS_COMMON) -O3 -g
CC_FLAGS_DEBUG_STATIC=$(CC_FLAGS_COMMON) -O1 -g -fno-omit-frame-pointer -fsanitize=address
CC_FLAGS_RELEASE_STATIC=$(CC_FLAGS_COMMON) -O3 -g
CC_FLAGS_DEBUG_EXECUTABLE=$(CC_FLAGS_COMMON) -O1 -g -fno-omit-frame-pointer -fsanitize=address
CC_FLAGS_RELEASE_EXECUTABLE=$(CC_FLAGS_COMMON) -O3 -g
CC_FLAG_OUTPUT=-o 

AR=ar
AR_FLAGS_COMMON=-r -s
AR_FLAGS_DEBUG_STATIC=$(AR_FLAGS_COMMON)
AR_FLAGS_RELEASE_STATIC=$(AR_FLAGS_COMMON)
AR_FLAG_OUTPUT=

LINK=ccache c++
LINK_FLAGS_COMMON=-g -Wl,-rpath=\$$ORIGIN -Wl,-gc-sections
LINK_FLAGS_DEBUG_SHARED=$(LINK_FLAGS_COMMON) -shared -fsanitize=address
LINK_FLAGS_RELEASE_SHARED=$(LINK_FLAGS_COMMON) -shared 
LINK_FLAGS_DEBUG_STATIC=$(LINK_FLAGS_COMMON) -fsanitize=address
LINK_FLAGS_RELEASE_STATIC=$(LINK_FLAGS_COMMON)
LINK_FLAGS_DEBUG_EXECUTABLE=$(LINK_FLAGS_COMMON) -fsanitize=address
LINK_FLAGS_RELEASE_EXECUTABLE=$(LINK_FLAGS_COMMON)
LINK_FLAG_OUTPUT=-o 
LINK_LIBS=

BINARY_INCLUDE=$(TRAKTOR_HOME)/bin/linux/releasestatic/Traktor.Run.App $(TRAKTOR_HOME)/scripts/binaryinclude/BinaryInclude.run
