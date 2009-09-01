#ifndef traktor_physics_RigidEntity_H
#define traktor_physics_RigidEntity_H

#include "Core/Heap/Ref.h"
#include "World/Entity/SpatialEntity.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class Body;

/*! \brief Rigid body entity.
 * \ingroup Physics
 */
class T_DLLCLASS RigidEntity : public world::SpatialEntity
{
	T_RTTI_CLASS(RigidEntity)

public:
	RigidEntity(
		Body* body,
		world::SpatialEntity* entity
	);

	virtual ~RigidEntity();

	virtual void destroy();

	virtual void update(const world::EntityUpdate* update);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb getBoundingBox() const;

	inline Body* getBody() const { return m_body; }

	inline world::SpatialEntity* getEntity() const { return m_entity; }

private:
	Ref< Body > m_body;
	Ref< world::SpatialEntity > m_entity;
};

	}
}

#endif	// traktor_physics_RigidEntity_H
