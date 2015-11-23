# Configuration file

# Toolchain and SDK configuration.
XCODE_PATH=$(shell xcode-select --print-path)
TOOLCHAIN_BIN=$(XCODE_PATH)/Toolchains/XcodeDefault.xctoolchain/usr/bin
SDK_SYSROOT=$(XCODE_PATH)/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator9.1.sdk
ARCH=x86_64
IPHONE_OS_MIN_VERSION=7.1

# Build tools.
CC=$(TOOLCHAIN_BIN)/clang

CC_FLAGS_COMMON=-arch $(ARCH) -fmessage-length=0 -fdiagnostics-show-note-include-stack -fmacro-backtrace-limit=0 -fstrict-aliasing -fvisibility=hidden -fpascal-strings -isysroot $(SDK_SYSROOT) -miphoneos-version-min=$(IPHONE_OS_MIN_VERSION)

#CC_FLAGS_WARNINGS=-Wno-trigraphs -Wno-missing-field-initializers -Wno-missing-prototypes -Wno-non-virtual-dtor -Wno-overloaded-virtual -Wno-exit-time-destructors -Wno-missing-braces -Wparentheses -Wswitch -Wno-unused-function -Wno-unused-label -Wno-unused-parameter -Wunused-variable -Wunused-value -Wno-empty-body -Wno-uninitialized -Wno-unknown-pragmas -Wno-shadow -Wno-four-char-constants -Wno-conversion -Wno-constant-conversion -Wno-int-conversion -Wno-bool-conversion -Wno-enum-conversion -Wshorten-64-to-32 -Wno-newline-eof -Wno-c++11-extensions -Wdeprecated-declarations -Winvalid-offsetof -Wno-sign-conversion
CC_FLAGS_WARNINGS=

CC_FLAGS_DEBUG_SHARED=$(CC_FLAGS_COMMON) $(CC_FLAGS_WARNINGS) -g -O0 -D _UNICODE -D UNICODE -D __IOS__ 
CC_FLAGS_RELEASE_SHARED=$(CC_FLAGS_COMMON) $(CC_FLAGS_WARNINGS) -g -O3 -D _UNICODE -D UNICODE -D __IOS__ 
CC_FLAGS_DEBUG_STATIC=$(CC_FLAGS_COMMON) $(CC_FLAGS_WARNINGS) -g -O0 -D _UNICODE -D UNICODE -D __IOS__ 
CC_FLAGS_RELEASE_STATIC=$(CC_FLAGS_COMMON) $(CC_FLAGS_WARNINGS) -g -O3 -D _UNICODE -D UNICODE -D __IOS__ 
CC_FLAGS_DEBUG_EXECUTABLE=$(CC_FLAGS_COMMON) $(CC_FLAGS_WARNINGS) -g -O0 -D _UNICODE -D UNICODE -D __IOS__ 
CC_FLAGS_RELEASE_EXECUTABLE=$(CC_FLAGS_COMMON) $(CC_FLAGS_WARNINGS) -g -O3 -D _UNICODE -D UNICODE -D __IOS__ 

MM=$(TOOLCHAIN_BIN)/clang

MM_FLAGS_COMMON=-arch $(ARCH) -fmessage-length=0 -fdiagnostics-show-note-include-stack -fmacro-backtrace-limit=0 -fstrict-aliasing -fvisibility=hidden -fpascal-strings -isysroot $(SDK_SYSROOT) -miphoneos-version-min=$(IPHONE_OS_MIN_VERSION)

#MM_FLAGS_WARNINGS=-Wno-trigraphs -Wno-missing-field-initializers -Wno-missing-prototypes -Wno-non-virtual-dtor -Wno-overloaded-virtual -Wno-exit-time-destructors -Wno-missing-braces -Wparentheses -Wswitch -Wno-unused-function -Wno-unused-label -Wno-unused-parameter -Wunused-variable -Wunused-value -Wno-empty-body -Wno-uninitialized -Wno-unknown-pragmas -Wno-shadow -Wno-four-char-constants -Wno-conversion -Wno-constant-conversion -Wno-int-conversion -Wno-bool-conversion -Wno-enum-conversion -Wshorten-64-to-32 -Wno-newline-eof -Wno-c++11-extensions -Wdeprecated-declarations -Winvalid-offsetof -Wno-sign-conversion
MM_FLAGS_WARNINGS=

MM_FLAGS_DEBUG_SHARED=$(CC_FLAGS_COMMON) $(CC_FLAGS_WARNINGS) -g -O0 -D _UNICODE -D UNICODE -D __IOS__ 
MM_FLAGS_RELEASE_SHARED=$(CC_FLAGS_COMMON) $(CC_FLAGS_WARNINGS) -g -O3 -D _UNICODE -D UNICODE -D __IOS__ 
MM_FLAGS_DEBUG_STATIC=$(CC_FLAGS_COMMON) $(CC_FLAGS_WARNINGS) -g -O0 -D _UNICODE -D UNICODE -D __IOS__ 
MM_FLAGS_RELEASE_STATIC=$(CC_FLAGS_COMMON) $(CC_FLAGS_WARNINGS) -g -O3 -D _UNICODE -D UNICODE -D __IOS__ 
MM_FLAGS_DEBUG_EXECUTABLE=$(CC_FLAGS_COMMON) $(CC_FLAGS_WARNINGS) -g -O0 -D _UNICODE -D UNICODE -D __IOS__ 
MM_FLAGS_RELEASE_EXECUTABLE=$(CC_FLAGS_COMMON) $(CC_FLAGS_WARNINGS) -g -O3 -D _UNICODE -D UNICODE -D __IOS__ 

AR=$(TOOLCHAIN_BIN)/libtool
AR_FLAGS_COMMON=-static -syslibroot $(SDK_SYSROOT)
AR_FLAGS_DEBUG_STATIC=$(AR_FLAGS_COMMON)
AR_FLAGS_RELEASE_STATIC=$(AR_FLAGS_COMMON)

LINK=$(TOOLCHAIN_BIN)/clang $(CC_FLAGS_COMMON)
LINK_LIBS=

ADDITIONAL_INCLUDE_PATHS=
ADDITIONAL_LIBRARY_PATHS=

BINARY_INCLUDE=BinaryInclude
