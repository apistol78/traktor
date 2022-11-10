/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

/*! \ingroup Core */
//@{

#if defined (_MSC_VER)

#	define T_MATH_ALIGN16 T_ALIGN16

#	if !defined(_DEBUG)
#		define T_MATH_USE_INLINE
#		define T_MATH_INLINE __forceinline
#		define T_MATH_USE_SSE2
#	else	// _DEBUG
#		define T_MATH_USE_SSE2
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
//#			elif !TARGET_OS_IPHONE
//#				define T_MATH_USE_SSE2
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
//#			elif !TARGET_OS_IPHONE
//#				define T_MATH_USE_SSE2
#			endif
#		elif defined(__LINUX__)
#			define T_MATH_USE_SSE2
#		elif defined(__ANDROID__) && defined(__ARM_NEON)
#			define T_MATH_USE_NEON
#		endif
#	endif

#endif	// __GNUC__

#if defined(T_MATH_USE_SSE2)
#	if defined(_MSC_VER) || defined(__APPLE__) || defined(__LINUX__)
#		include <xmmintrin.h>
#		include <smmintrin.h>
#		include <immintrin.h>
#	endif
#endif

#if defined(T_MATH_USE_ALTIVEC)
#	include <altivec.h>
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

