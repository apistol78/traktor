#pragma once
#ifndef traktor_Config_H
#define traktor_Config_H

/*! \ingroup Core */
//@{

#if defined (_MSC_VER)
#	if !defined (T_STATIC) && !defined (__CLR_VER)
#		define T_DLLIMPORT __declspec(dllimport)
#		define T_DLLEXPORT __declspec(dllexport)
#	endif
#	define T_NOVTABLE __declspec(novtable)
#	define T_RESTRICT __restrict
#	define T_FORCE_INLINE __forceinline
#	define T_ALIGN16 __declspec(align(16))
#	define T_NOEXCEPT noexcept
#elif defined(__GNUC__) || defined(__ANDROID__) || defined(__PS3__)
#	if !defined(T_STATIC)
#		define T_DLLIMPORT __attribute__((visibility("default")))
#		define T_DLLEXPORT __attribute__((visibility("default")))
#		define T_DLLLOCAL __attribute__((visibility("hidden")))
#	endif
#	define T_FORCE_INLINE inline
#	define T_ALIGN16 __attribute__((aligned(16)))
#	if !defined(__APPLE__)
#		define T_NOEXCEPT noexcept
#	endif
#	if defined(__PS3__) && defined(SPU)
#		define nullptr 0
#	endif
#endif

#if !defined(T_DLLIMPORT)
#	define T_DLLIMPORT
#endif
#if !defined(T_DLLEXPORT)
#	define T_DLLEXPORT
#endif
#if !defined(T_DLLLOCAL)
#	define T_DLLLOCAL
#endif
#if !defined(T_NOVTABLE)
#	define T_NOVTABLE
#endif
#if !defined(T_RESTRICT)
#	define T_RESTRICT
#endif
#if !defined(T_FORCE_INLINE)
#	define T_FORCE_INLINE
#endif
#if !defined(T_ALIGN16)
#	define T_ALIGN16
#endif
#if !defined(T_NOEXCEPT)
#	define T_NOEXCEPT
#endif
#if !defined(T_UNALIGNED)
#	define T_UNALIGNED
#endif

// Prevent class from being copyable.
#define T_NO_COPY_CLASS(NAME)					\
	NAME(NAME const &) = delete;				\
	NAME(NAME &&) = delete;						\
	void operator = (NAME const &) = delete;

// Detect endian.
#if defined(__APPLE__)
// Mac has a special header defining target endian.
#	include <TargetConditionals.h>
#	if TARGET_RT_LITTLE_ENDIAN
#		define T_LITTLE_ENDIAN
#	elif TARGET_RT_BIG_ENDIAN
#		define T_BIG_ENDIAN
#	endif
#elif defined(_XBOX) || defined(__PS3__)
// Both Xenon and PS3 use a PowerPC derivate.
#	define T_BIG_ENDIAN
#elif defined(_WIN32) || defined(__PS3__) || defined(_XBOX_ONE)
// Assume little endian on Win32 as it's probably most common.
#	define T_LITTLE_ENDIAN
#elif defined(__GNUC__)
// Assume little endian on unknown platform using GCC, assuming Linux/x86 for now.
#	define T_LITTLE_ENDIAN
#endif

// Disable various compiler warnings.
#if defined(_MSC_VER)
#	pragma warning( disable : 4275 )	// non-dll class foo used as base for dll-interface class bar
#	pragma warning( disable : 4251 )	// <identifier> : class foo needs to have dll-interface to be used by clients of class bar
#	pragma warning( disable : 4190 )	// 'foo' has C-linkage specified, but returns UDT 'bar' which is incompatible with C
#	pragma warning( disable : 4345 )	// behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized
#endif

// Cross platform type definitions.
#if defined(_MSC_VER)
#	if !defined(T_HAVE_TYPES)
#		if _MSC_VER >= 1700
#			include <stdint.h>
#		else

typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;

typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;

typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

#		endif
#		define T_HAVE_TYPES
#	endif
#else
#	if !defined(T_HAVE_TYPES)
#		if defined(__PS3__)
#			include <sys/types.h>
#		else
#			include <stdint.h>
#		endif
#		define T_HAVE_TYPES
#	endif
#	include <stddef.h>
#endif

// Determine size of array in number of elements.
#if !defined(sizeof_array)
#	define sizeof_array(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

// Use these macros in c/d tors only.
#if defined(__ANDROID__) || defined(__PS3__)
#	define T_EXCEPTION_GUARD_BEGIN
#	define T_EXCEPTION_GUARD_END
#endif
#if !defined(T_EXCEPTION_GUARD_BEGIN)
#	define T_EXCEPTION_GUARD_BEGIN try {
#endif
#if !defined(T_EXCEPTION_GUARD_END)
#	if defined(_DEBUG)
#		define T_EXCEPTION_GUARD_END } catch (...) { T_BREAKPOINT; }
#	else
#		define T_EXCEPTION_GUARD_END } catch (...) { }
#	endif
#endif

// Anonymous variable
#if !defined(T_ANONYMOUS_VAR)
#	define T_ANONYMOUS_VAR_2(x,y) x ## y
#	define T_ANONYMOUS_VAR_1(x,y) T_ANONYMOUS_VAR_2(x,y)
#	define T_ANONYMOUS_VAR(T) T T_ANONYMOUS_VAR_1(_anonymous, __LINE__)
#endif

#if !defined(T_FILE_LINE)
#	define T_FILE_LINE_2(x) #x
#	define T_FILE_LINE_1(x) T_FILE_LINE_2(x)
#	define T_FILE_LINE __FILE__ "(" T_FILE_LINE_1(__LINE__) ")"
#	define T_FILE_LINE_W_2(x) L##x
#	define T_FILE_LINE_W_1(x) T_FILE_LINE_W_2(x)
#	define T_FILE_LINE_W T_FILE_LINE_W_1(T_FILE_LINE)
#endif

// Include assert helper.
#include "Core/Assert.h"

//@}

#endif	// traktor_Config_H
