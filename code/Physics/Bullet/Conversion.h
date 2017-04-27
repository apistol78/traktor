/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_Conversion_H
#define traktor_physics_Conversion_H

#include <LinearMath/btMatrix3x3.h>
#include <LinearMath/btTransform.h>
#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>
#include "Core/Math/Float.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Transform.h"

namespace traktor
{
	namespace physics
	{

#if !defined(T_CONVERT_ASSERT)
#	define T_CONVERT_ASSERT T_ASSERT
#endif

/*! \ingroup Bullet */
//@{

/*! \brief Convert from Bullet vector. */
inline Vector4 fromBtVector3(const btVector3& v, float w)
{
	T_CONVERT_ASSERT (!isNanOrInfinite((v).x()));
	T_CONVERT_ASSERT (!isNanOrInfinite((v).y()));
	T_CONVERT_ASSERT (!isNanOrInfinite((v).z()));
	T_CONVERT_ASSERT (!isNanOrInfinite(w));
	return Vector4(v.x(), v.y(), v.z(), w);
}

/*! \brief Convert to Bullet vector. */
inline btVector3 toBtVector3(const Vector4& v)
{
	T_CONVERT_ASSERT (!isNanOrInfinite((v).x()));
	T_CONVERT_ASSERT (!isNanOrInfinite((v).y()));
	T_CONVERT_ASSERT (!isNanOrInfinite((v).z()));
	return btVector3(v.x(), v.y(), v.z());
}

/*! \brief Convert from Bullet quaternion. */
inline Quaternion fromBtQuaternion(const btQuaternion& q)
{
	return Quaternion(q.x(), q.y(), q.z(), q.w());
}

/*! \brief Convert to Bullet quaternion. */
inline btQuaternion toBtQuaternion(const Quaternion& q)
{
	return btQuaternion(q.e.x(), q.e.y(), q.e.z(), q.e.w());
}

/*! \brief Convert from Bullet matrix. */
inline Matrix33 fromBtMatrix(const btMatrix3x3& matrix)
{
	return Matrix33(
		matrix.getRow(0).x(), matrix.getRow(0).y(), matrix.getRow(0).z(),
		matrix.getRow(1).x(), matrix.getRow(1).y(), matrix.getRow(1).z(),
		matrix.getRow(2).x(), matrix.getRow(2).y(), matrix.getRow(2).z()
	);
}

/*! \brief Convert from Bullet transform. */
inline Transform fromBtTransform(const btTransform& transform)
{
	return Transform(
		fromBtVector3(transform.getOrigin(), 0.0f),
		fromBtQuaternion(transform.getRotation())
	);
}

/*! \brief Convert to Bullet transform. */
inline btTransform toBtTransform(const Transform& transform)
{
	btQuaternion basis = toBtQuaternion(transform.rotation());
	btVector3 origin = toBtVector3(transform.translation());
	return btTransform(basis, origin);
}

//@}

	}
}

#endif	// traktor_physics_Conversion_H
