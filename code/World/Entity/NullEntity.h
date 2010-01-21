#ifndef traktor_world_NullEntity_H
#define traktor_world_NullEntity_H

#include "World/Entity/SpatialEntity.h"

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

/*! \brief Null entity.
 * \ingroup World
 */
class T_DLLCLASS NullEntity : public SpatialEntity
{
	T_RTTI_CLASS;

public:
	NullEntity(const Transform& transform);

	virtual void update(const EntityUpdate* update);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb getBoundingBox() const;

private:
	Transform m_transform;
};

	}
}

#endif	// traktor_world_NullEntity_H
