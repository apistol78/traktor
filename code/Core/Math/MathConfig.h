#ifndef traktor_MathConfig_H
#define traktor_MathConfig_H

/*! \ingroup Core */
//@{

#if defined (_MSC_VER)

#	define T_MATH_ALIGN16 T_ALIGN16

#	if !defined(_DEBUG)
#		define T_MATH_USE_INLINE
#		define T_MATH_INLINE __forceinline
#		if !defined(WINCE)
#			define T_MATH_USE_SSE2
#		endif
#	else	// _DEBUG
#		if !defined(WINCE)
#			define T_MATH_USE_SSE2
#		endif
#	endif

#endif	// _MSC_VER

#if defined (__GNUC__)

#	define T_MATH_ALIGN16 T_ALIGN16

#	if !defined(_DEBUG)
#		define T_MATH_USE_INLINE
#		define T_MATH_INLINE inline
#		if defined(__APPLE__) && defined(TARGET_OS_MAC)
#			define T_MATH_USE_SSE2
#		endif
#	else	// _DEBUG
#		if defined(__APPLE__) && defined(TARGET_OS_MAC)
#			define T_MATH_USE_SSE2
#		endif
#	endif

#endif	// __GNUC__

#if defined(T_MATH_USE_SSE2)
#	if defined(_MSC_VER) || defined(__APPLE__)
#		include <xmmintrin.h>
#	endif
#endif

#if !defined(T_MATH_ALIGN16)
#	define T_MATH_ALIGN16
#endif
#if !defined(T_MATH_INLINE)
#	define T_MATH_INLINE
#endif

//@}

#endif	// traktor_MathConfig_H
