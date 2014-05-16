#ifndef traktor_spray_PlaneCollisionModifier_H
#define traktor_spray_PlaneCollisionModifier_H

#include "Core/Math/Plane.h"
#include "Spray/Modifier.h"

namespace traktor
{
	namespace spray
	{

/*! \brief Plane collision modifier.
 * \ingroup Spray
 */
class PlaneCollisionModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	PlaneCollisionModifier(const Plane& plane, float radius, float restitution);

#if defined(T_MODIFIER_USE_PS3_SPURS)
	virtual void update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const;
#else
	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const;
#endif

private:
	Plane m_plane;
	Scalar m_radius;
	Scalar m_restitution;
};

	}
}

#endif	// traktor_spray_PlaneCollisionModifier_H
