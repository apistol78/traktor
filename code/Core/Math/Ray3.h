/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! 3D ray
 * \ingroup Core
 */
class T_DLLCLASS Ray3
{
public:
	Vector4 origin;
	Vector4 direction;

	T_MATH_INLINE Ray3();

	T_MATH_INLINE Ray3(const Vector4& origin, const Vector4& direction);

	T_MATH_INLINE Scalar distance(const Vector4& pt) const;

	T_MATH_INLINE Vector4 operator * (const Scalar& k) const;
};

}

#if defined(T_MATH_USE_INLINE)
#	include "Core/Math/Std/Ray3.inl"
#endif

