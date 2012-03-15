#ifndef traktor_physics_RigidEntity_H
#define traktor_physics_RigidEntity_H

#include "World/Entity/Entity.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class Body;

/*! \brief Rigid body entity.
 * \ingroup Physics
 */
class T_DLLCLASS RigidEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	RigidEntity(
		Body* body,
		world::Entity* entity
	);

	virtual ~RigidEntity();

	virtual void destroy();

	virtual void update(const world::EntityUpdate* update);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

	Body* getBody() const { return m_body; }

	world::Entity* getEntity() const { return m_entity; }

private:
	Ref< Body > m_body;
	Ref< world::Entity > m_entity;
};

	}
}

#endif	// traktor_physics_RigidEntity_H
