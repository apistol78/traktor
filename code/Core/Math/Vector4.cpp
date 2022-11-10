/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/MathConfig.h"

#if !defined(T_MATH_USE_INLINE)
#	if defined(T_MATH_USE_SSE2)
#		include "Core/Math/Sse2/Vector4.inl"
#	elif defined(T_MATH_USE_ALTIVEC)
#		include "Core/Math/AltiVec/Vector4.inl"
#	elif defined(T_MATH_USE_NEON)
#		include "Core/Math/Neon/Vector4.inl"
#	else
#		include "Core/Math/Std/Vector4.inl"
#	endif
#endif
