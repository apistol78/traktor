#ifndef traktor_spray_PlaneCollisionModifier_H
#define traktor_spray_PlaneCollisionModifier_H

#include "Spray/Modifier.h"
#include "Core/Math/Plane.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS(PlaneCollisionModifier)

public:
	PlaneCollisionModifier();

	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const;

	virtual bool serialize(Serializer& s);

private:
	Plane m_plane;
	Scalar m_restitution;
};

	}
}

#endif	// traktor_spray_PlaneCollisionModifier_H
