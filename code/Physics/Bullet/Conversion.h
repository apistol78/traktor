#ifndef traktor_physics_Conversion_H
#define traktor_physics_Conversion_H

#include "Core/Math/Float.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Transform.h"

namespace traktor
{
	namespace physics
	{

/*! \ingroup Bullet */
//@{

/*! \brief Convert from Bullet vector. */
inline Vector4 fromBtVector3(const btVector3& v, float w)
{
	T_ASSERT (!isNan((v).x()));
	T_ASSERT (!isNan((v).y()));
	T_ASSERT (!isNan((v).z()));
	T_ASSERT (!isNan(w));
	return Vector4::loadAligned(v.m_floats).xyz0() + Vector4(0.0f, 0.0f, 0.0f, w);
}

/*! \brief Convert to Bullet vector. */
inline btVector3 toBtVector3(const Vector4& v)
{
	T_ASSERT (!isNan((v).x()));
	T_ASSERT (!isNan((v).y()));
	T_ASSERT (!isNan((v).z()));

	btVector3 vr;
	v.storeAligned(vr.m_floats);
	return vr;
}

/*! \brief Convert from Bullet quaternion. */
inline Quaternion fromBtQuaternion(const btQuaternion& q)
{
	return Quaternion(q.x(), q.y(), q.z(), q.w());
}

/*! \brief Convert to Bullet quaternion. */
inline btQuaternion toBtQuaternion(const Quaternion& q)
{
	float T_MATH_ALIGN16 e[4];
	q.e.storeAligned(e);
	return btQuaternion(e[0], e[1], e[2], e[3]);
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
