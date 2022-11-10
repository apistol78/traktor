/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

/*! Convert from Bullet vector. */
inline Vector4 fromBtVector3(const btVector3& v, float w)
{
	T_CONVERT_ASSERT(!isNanOrInfinite((v).x()));
	T_CONVERT_ASSERT(!isNanOrInfinite((v).y()));
	T_CONVERT_ASSERT(!isNanOrInfinite((v).z()));
	T_CONVERT_ASSERT(!isNanOrInfinite(w));
	return Vector4(v.x(), v.y(), v.z(), w);
}

/*! Convert to Bullet vector. */
inline btVector3 toBtVector3(const Vector4& v)
{
	T_CONVERT_ASSERT(!isNanOrInfinite((v).x()));
	T_CONVERT_ASSERT(!isNanOrInfinite((v).y()));
	T_CONVERT_ASSERT(!isNanOrInfinite((v).z()));
	return btVector3(v.x(), v.y(), v.z());
}

/*! Convert from Bullet quaternion. */
inline Quaternion fromBtQuaternion(const btQuaternion& q)
{
	return Quaternion(q.x(), q.y(), q.z(), q.w());
}

/*! Convert to Bullet quaternion. */
inline btQuaternion toBtQuaternion(const Quaternion& q)
{
	return btQuaternion(q.e.x(), q.e.y(), q.e.z(), q.e.w());
}

/*! Convert from Bullet matrix. */
inline Matrix33 fromBtMatrix(const btMatrix3x3& matrix)
{
	return Matrix33(
		matrix.getRow(0).x(), matrix.getRow(0).y(), matrix.getRow(0).z(),
		matrix.getRow(1).x(), matrix.getRow(1).y(), matrix.getRow(1).z(),
		matrix.getRow(2).x(), matrix.getRow(2).y(), matrix.getRow(2).z()
	);
}

/*! Convert from Bullet transform. */
inline Transform fromBtTransform(const btTransform& transform)
{
	return Transform(
		fromBtVector3(transform.getOrigin(), 0.0f),
		fromBtQuaternion(transform.getRotation())
	);
}

/*! Convert to Bullet transform. */
inline btTransform toBtTransform(const Transform& transform)
{
	btQuaternion basis = toBtQuaternion(transform.rotation());
	btVector3 origin = toBtVector3(transform.translation());
	return btTransform(basis, origin);
}

//@}

	}
}

