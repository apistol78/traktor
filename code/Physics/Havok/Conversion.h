/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_Conversion_H
#define traktor_physics_Conversion_H

#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Transform.h"

namespace traktor
{
	namespace physics
	{

/*! \ingroup Havok */
//@{

/*! \brief Convert from Havok vector. */
T_FORCE_INLINE Vector4 fromHkVector4(const hkVector4& v)
{
	return Vector4(v(0), v(1), v(2), v(3));
}

/*! \brief Convert to Havok vector. */
T_FORCE_INLINE hkVector4 toHkVector4(const Vector4& v)
{
	return hkVector4(v.x(), v.y(), v.z(), v.w());
}

/*! \brief Convert from Havok quaternion. */
T_FORCE_INLINE Quaternion fromHkQuaternion(const hkQuaternion& q)
{
	return Quaternion(q.m_vec(0), q.m_vec(1), q.m_vec(2), q.m_vec(3));
}

/*! \brief Convert to Havok quaternion. */
T_FORCE_INLINE hkQuaternion toHkQuaternion(const Quaternion& q)
{
	return hkQuaternion(q.e.x(), q.e.y(), q.e.z(), q.e.w());
}

/*! \brief Convert from Havok matrix. */
T_FORCE_INLINE Matrix33 fromHkMatrix3(const hkMatrix3& m)
{
	Matrix33 out;
	for (int row = 0; row < 3; ++row)
	{
		for (int col = 0; col < 3; ++col)
		{
			out.e[row][col] = m(row, col);
		}
	}
	return out;
}

/*! \brief Convert to Havok matrix. */
T_FORCE_INLINE hkMatrix3 toHkMatrix3(const Matrix33& m)
{
	hkMatrix3 out;
	for (int row = 0; row < 3; ++row)
	{
		for (int col = 0; col < 3; ++col)
		{
			out(row, col) = m.e[row][col];
		}
	}
	return out;
}

/*! \brief Convert from Havok transform. */
T_FORCE_INLINE Transform fromHkTransform(const hkTransform& t)
{
	hkVector4 hkt = t.getTranslation();
	hkQuaternion hkr = hkQuaternion(t.getRotation());
	return Transform(fromHkVector4(hkt), fromHkQuaternion(hkr));
}

/*! \brief Convert to Havok transform. */
T_FORCE_INLINE hkTransform toHkTransform(const Transform& m)
{
	return hkTransform(
		toHkQuaternion(m.rotation()),
		toHkVector4(m.translation())
	);
}

//@}

	}
}

#endif	// traktor_physics_Conversion_H
