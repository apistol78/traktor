/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
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
