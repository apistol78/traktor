#pragma once

#include "Core/Ref.h"
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
class EntityEventManager;

	}

	namespace physics
	{

class Body;
struct CollisionInfo;

/*! Rigid body entity.
 * \ingroup Physics
 */
class T_DLLCLASS RigidEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	RigidEntity(
		Body* body,
		world::Entity* entity,
		world::EntityEventManager* eventManager,
		world::IEntityEvent* eventCollide
	);

	virtual ~RigidEntity();

	virtual void destroy() override final;

	virtual void update(const world::UpdateParams& update) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Transform getTransform() const override final;

	virtual Aabb3 getBoundingBox() const override final;

	Body* getBody() const { return m_body; }

	world::Entity* getEntity() const { return m_entity; }

private:
	Ref< Body > m_body;
	Ref< world::Entity > m_entity;
	Ref< world::EntityEventManager > m_eventManager;
	Ref< world::IEntityEvent > m_eventCollide;

	void collisionListener(const physics::CollisionInfo& collisionInfo);
};

	}
}

