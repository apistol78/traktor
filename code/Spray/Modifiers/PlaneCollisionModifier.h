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
	PlaneCollisionModifier(const Plane& plane, float radius, float restitution);

#if defined(T_MODIFIER_USE_PS3_SPURS)
	virtual void update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const override final;
#else
	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const override final;
#endif

private:
	Plane m_plane;
	Scalar m_radius;
	Scalar m_restitution;
};

	}
}

