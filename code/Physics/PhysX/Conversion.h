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
T_FORCE_INLINE Vector4 fromNxVec3(const NxVec3& v, float w = 0.0f)
{
	return Vector4(v[0], v[1], v[2], w);
}

/*! \brief Convert to PhysX vector. */
T_FORCE_INLINE NxVec3 toNxVec3(const Vector4& v)
{
	return NxVec3(v.x(), v.y(), v.z());
}

/*! \brief Convert from PhysX matrix. */
T_FORCE_INLINE Matrix33 fromNxMat33(const NxMat33& m)
{
	return Matrix33(
		m(0, 0), m(0, 1), m(0, 2),
		m(1, 0), m(1, 1), m(1, 2),
		m(2, 0), m(2, 1), m(2, 2)
	);
}

/*! \brief Convert from PhysX matrix. */
T_FORCE_INLINE Transform fromNxMat34(const NxMat34& m)
{
	return Transform(Matrix44(
		m.M(0, 0), m.M(0, 1), m.M(0, 2), m.t[0],
		m.M(1, 0), m.M(1, 1), m.M(1, 2), m.t[1],
		m.M(2, 0), m.M(2, 1), m.M(2, 2), m.t[2],
		0.0f, 0.0f, 0.0f, 1.0f
	));
}

/*! \brief Convert to PhysX matrix. */
T_FORCE_INLINE NxMat34 toNxMat34(const Transform& t)
{
	Matrix44 m = t.toMatrix44();
	NxMat34 r;
	r.M(0, 0) = m(0, 0); r.M(1, 0) = m(1, 0); r.M(2, 0) = m(2, 0);
	r.M(0, 1) = m(0, 1); r.M(1, 1) = m(1, 1); r.M(2, 1) = m(2, 1);
	r.M(0, 2) = m(0, 2); r.M(1, 2) = m(1, 2); r.M(2, 2) = m(2, 2);
	r.t[0] = m(0, 3); r.t[1] = m(1, 3); r.t[2] = m(2, 3);
	return r;
}

//@}

	}
}

#endif	// traktor_physics_Conversion_H
