#ifndef traktor_spray_PlaneCollisionModifier_H
#define traktor_spray_PlaneCollisionModifier_H

#include "Core/Math/Plane.h"
#include "Spray/Modifier.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

/*! \brief Plane collision modifier.
 * \ingroup Spray
 */
class T_DLLCLASS PlaneCollisionModifier : public Modifier
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
