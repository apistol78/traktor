#ifndef traktor_physics_Conversion_H
#define traktor_physics_Conversion_H

#include "Core/Math/Float.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Matrix44.h"

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
	return Vector4(v.x(), v.y(), v.z(), w);
}

/*! \brief Convert to Bullet vector. */
inline btVector3 toBtVector3(const Vector4& v)
{
	T_ASSERT (!isNan((v).x()));
	T_ASSERT (!isNan((v).y()));
	T_ASSERT (!isNan((v).z()));
	return btVector3(v.x(), v.y(), v.z());
}

/*! \brief Convert from Bullet matrix. */
inline Matrix33 fromBtMatrix(const btMatrix3x3& matrix)
{
	return Matrix33(
		matrix.getRow(0).x(), matrix.getRow(1).x(), matrix.getRow(2).x(),
		matrix.getRow(0).y(), matrix.getRow(1).y(), matrix.getRow(2).y(),
		matrix.getRow(0).z(), matrix.getRow(1).z(), matrix.getRow(2).z()
	);
}

/*! \brief Convert from Bullet transform. */
inline Matrix44 fromBtTransform(const btTransform& transform)
{
	const btMatrix3x3& basis = transform.getBasis();
	const btVector3& origin = transform.getOrigin();

	return Matrix44(
		basis.getRow(0).x(), basis.getRow(1).x(), basis.getRow(2).x(), 0.0f,
		basis.getRow(0).y(), basis.getRow(1).y(), basis.getRow(2).y(), 0.0f,
		basis.getRow(0).z(), basis.getRow(1).z(), basis.getRow(2).z(), 0.0f,
		origin.x(), origin.y(), origin.z(), 1.0f
	);
}

/*! \brief Convert to Bullet transform. */
inline btTransform toBtTransform(const Matrix44& transform)
{
	btMatrix3x3 basis(
		transform.axisX().x(), transform.axisY().x(), transform.axisZ().x(),
		transform.axisX().y(), transform.axisY().y(), transform.axisZ().y(),
		transform.axisX().z(), transform.axisY().z(), transform.axisZ().z()
	);
	btVector3 origin = toBtVector3(transform.translation());
	return btTransform(basis, origin);
}

//@}

	}
}

#endif	// traktor_physics_Conversion_H
