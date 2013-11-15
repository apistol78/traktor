#ifndef traktor_world_GodRayEntity_H
#define traktor_world_GodRayEntity_H

#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Point light entity.
 * \ingroup World
 */
class T_DLLCLASS GodRayEntity : public Entity
{
	T_RTTI_CLASS;

public:
	GodRayEntity(const Transform& transform);

	virtual void update(const UpdateParams& update);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

private:
	Transform m_transform;
};

	}
}

#endif	// traktor_world_GodRayEntity_H
