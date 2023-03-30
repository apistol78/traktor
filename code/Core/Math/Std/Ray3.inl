/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Ray3.h"

namespace traktor
{

T_MATH_INLINE Ray3::Ray3()
{
}

T_MATH_INLINE Ray3::Ray3(const Vector4& origin_, const Vector4& direction_)
:	origin(origin_)
,	direction(direction_)
{
}

T_MATH_INLINE Scalar Ray3::distance(const Vector4& pt) const
{
	return cross(direction, origin - pt).length();
}

T_MATH_INLINE Vector4 Ray3::operator * (const Scalar& k) const
{
	return origin + direction * k;
}

}
