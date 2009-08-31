#ifndef traktor_physics_Conversion_H
#define traktor_physics_Conversion_H

#include "Core/Math/Vector4.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Matrix44.h"

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
T_FORCE_INLINE Matrix44 fromHkTransform(const hkTransform& t)
{
	float T_ALIGN16 e[4 * 4];
	t.get4x4ColumnMajor(e);
	Matrix44 m;
	m.load(e);
	return m;
}

/*! \brief Convert to Havok transform. */
T_FORCE_INLINE hkTransform toHkTransform(const Matrix44& m)
{
	float T_ALIGN16 e[4 * 4];
	m.store(e);

	hkTransform t;
	t.set4x4ColumnMajor(e);

	return t;
}

//@}

	}
}

#endif	// traktor_physics_Conversion_H
