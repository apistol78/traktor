#ifndef traktor_physics_RigidEntity_H
#define traktor_physics_RigidEntity_H

#include "World/Entity.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityEvent;
class IEntityEventManager;

	}

	namespace physics
	{

class Body;
struct CollisionInfo;

/*! \brief Rigid body entity.
 * \ingroup Physics
 */
class T_DLLCLASS RigidEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	RigidEntity(
		Body* body,
		world::Entity* entity,
		world::IEntityEventManager* eventManager,
		world::IEntityEvent* eventCollide
	);

	virtual ~RigidEntity();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool getTransform(Transform& outTransform) const T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	Body* getBody() const { return m_body; }

	world::Entity* getEntity() const { return m_entity; }

private:
	Ref< Body > m_body;
	Ref< world::Entity > m_entity;
	Ref< world::IEntityEventManager > m_eventManager;
	Ref< world::IEntityEvent > m_eventCollide;

	void collisionListener(const physics::CollisionInfo& collisionInfo);
};

	}
}

#endif	// traktor_physics_RigidEntity_H
