#include "Core/Math/MathConfig.h"

#if !defined(T_MATH_USE_INLINE)
#	if defined(T_MATH_USE_SSE2)
#		include "Core/Math/Sse2/Scalar.inl"
#	else
#		include "Core/Math/Std/Scalar.inl"
#	endif
#endif
