/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/MathConfig.h"

#if !defined(T_MATH_USE_INLINE)
#	if defined(T_MATH_USE_SSE2)
#		include "Core/Math/Sse2/Scalar.inl"
#	elif defined(T_MATH_USE_ALTIVEC)
#		include "Core/Math/AltiVec/Scalar.inl"
#	elif defined(T_MATH_USE_ALTIVEC_SPU)
#		include "Core/Math/AltiVec/Ps3/Spu/Scalar.inl"
#	elif defined(T_MATH_USE_NEON)
#		include "Core/Math/Neon/Scalar.inl"
#	else
#		include "Core/Math/Std/Scalar.inl"
#	endif
#endif
