/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_MathConfig_H
#define traktor_MathConfig_H

/*! \ingroup Core */
//@{

#if defined (_MSC_VER)

#	define T_MATH_ALIGN16 T_ALIGN16

#	if !defined(_DEBUG)
#		define T_MATH_USE_INLINE
#		define T_MATH_INLINE __forceinline
#		if !defined(_XBOX)
#			define T_MATH_USE_SSE2
#		endif
#	else	// _DEBUG
#		if !defined(_XBOX)
#			define T_MATH_USE_SSE2
#		endif
#	endif

#endif	// _MSC_VER

#if defined(__GNUC__)
#	define T_MATH_ALIGN16 T_ALIGN16

#	if defined(__APPLE__)
#		include <TargetConditionals.h>
#	endif

#	if !defined(_DEBUG)
#		define T_MATH_USE_INLINE
#		define T_MATH_INLINE inline
#		if defined(__APPLE__)
#			if TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
#				define T_MATH_USE_NEON
#			elif !TARGET_OS_IPHONE
#				define T_MATH_USE_SSE2
#			endif
#		elif defined(__LINUX__)
#			define T_MATH_USE_SSE2
#		elif defined(__ANDROID__) && defined(__ARM_NEON)
#			define T_MATH_USE_NEON
#		endif
#	else	// _DEBUG
#		if defined(__APPLE__)
#			if TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
#				define T_MATH_USE_NEON
#			elif !TARGET_OS_IPHONE
#				define T_MATH_USE_SSE2
#			endif
#		elif defined(__LINUX__)
#			define T_MATH_USE_SSE2
#		elif defined(__ANDROID__) && defined(__ARM_NEON)
#			define T_MATH_USE_NEON
#		endif
#	endif

#endif	// __GNUC__

#if defined(_PS3)

#	define T_MATH_ALIGN16 T_ALIGN16

#	if !defined(SPU)
#		if !defined(_DEBUG)
#			define T_MATH_USE_INLINE
#			define T_MATH_INLINE inline
#			define T_MATH_USE_ALTIVEC
#		else	// _DEBUG
#			define T_MATH_USE_ALTIVEC
#		endif
#	else
#		if !defined(_DEBUG)
#			define T_MATH_USE_INLINE
#			define T_MATH_INLINE inline
#			define T_MATH_USE_ALTIVEC_SPU
#		else	// _DEBUG
#			define T_MATH_USE_ALTIVEC_SPU
#		endif
#	endif

#endif	// _PS3

#if defined(T_MATH_USE_SSE2)
#	if defined(_MSC_VER) || defined(__APPLE__) || defined(__LINUX__)
#		include <xmmintrin.h>
#	endif
#endif

#if defined(T_MATH_USE_ALTIVEC) || defined(T_MATH_USE_ALTIVEC_SPU)
#	if defined(_PS3)
#		include <vec_types.h>
#	else
#		include <altivec.h>
#	endif
#endif

#if defined(T_MATH_USE_NEON)
#	include <arm_neon.h>
#endif

#if !defined(T_MATH_ALIGN16)
#	define T_MATH_ALIGN16
#endif
#if !defined(T_MATH_INLINE)
#	define T_MATH_INLINE
#endif

//@}

#endif	// traktor_MathConfig_H
