/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Ray3_H
#define traktor_Ray3_H

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

/*! \brief 3D ray
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

#endif	// traktor_Ray3_H
