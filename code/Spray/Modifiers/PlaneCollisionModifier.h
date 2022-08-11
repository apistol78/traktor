#pragma once

#include "Core/Math/Plane.h"
#include "Spray/Modifier.h"

namespace traktor
{
	namespace spray
	{

/*! Plane collision modifier.
 * \ingroup Spray
 */
class PlaneCollisionModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	explicit PlaneCollisionModifier(const Plane& plane, float radius, float restitution);

	virtual void update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const override final;

private:
	Plane m_plane;
	Scalar m_radius;
	Scalar m_restitution;
};

	}
}

