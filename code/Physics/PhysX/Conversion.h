/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_Conversion_H
#define traktor_physics_Conversion_H

#include "Core/Math/Matrix33.h"
#include "Core/Math/Transform.h"

namespace traktor
{
	namespace physics
	{

/*! \ingroup PhysX */
//@{

/*! \brief Convert from PhysX vector. */
T_FORCE_INLINE Vector4 fromPxVec3(const physx::PxVec3& v, float w = 0.0f)
{
	return Vector4(v[0], v[1], v[2], w);
}

/*! \brief Convert to PhysX vector. */
T_FORCE_INLINE physx::PxVec3 toPxVec3(const Vector4& v)
{
	return physx::PxVec3(v.x(), v.y(), v.z());
}

/*! \brief Convert from PhysX matrix. */
T_FORCE_INLINE Matrix33 fromPxMat33(const physx::PxMat33& m)
{
	return Matrix33(
		m(0, 0), m(0, 1), m(0, 2),
		m(1, 0), m(1, 1), m(1, 2),
		m(2, 0), m(2, 1), m(2, 2)
	);
}

/*! \brief Convert from PhysX matrix. */
T_FORCE_INLINE Transform fromPxMat44(const physx::PxMat44& m)
{
	return Transform(Matrix44(
		m[0][0], m[0][1], m[0][2], m[0][3],
		m[1][0], m[1][1], m[1][2], m[1][3],
		m[2][0], m[2][1], m[2][2], m[2][3],
		m[3][0], m[3][1], m[3][2], m[3][3]
	));
}

/*! \brief Convert to PhysX matrix. */
T_FORCE_INLINE physx::PxMat44 toPxMat44(const Transform& t)
{
	Matrix44 m = t.toMatrix44();
	return physx::PxMat44(
		physx::PxVec4(m(0, 0), m(1, 0), m(2, 0), m(3, 0)),
		physx::PxVec4(m(0, 1), m(1, 1), m(2, 1), m(3, 1)),
		physx::PxVec4(m(0, 2), m(1, 2), m(2, 2), m(3, 2)),
		physx::PxVec4(m(0, 3), m(1, 3), m(2, 3), m(3, 3))
	);
}

T_FORCE_INLINE Quaternion fromPxQuat(const physx::PxQuat& q)
{
	return Quaternion(q.x, q.y, q.z, q.w);
}

T_FORCE_INLINE physx::PxQuat toPxQuat(const Quaternion& q)
{
	return physx::PxQuat(q.e.x(), q.e.y(), q.e.z(), q.e.w());
}

T_FORCE_INLINE Transform fromPxTransform(const physx::PxTransform& t)
{
	return Transform(
		fromPxVec3(t.p),
		fromPxQuat(t.q)
	);
}

T_FORCE_INLINE physx::PxTransform toPxTransform(const Transform& t)
{
	return physx::PxTransform(
		toPxVec3(t.translation()),
		toPxQuat(t.rotation())
	);
}

//@}

	}
}

#endif	// traktor_physics_Conversion_H
