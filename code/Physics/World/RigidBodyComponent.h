#pragma once

#include "World/IEntityComponent.h"

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

class Entity;
class IEntityEvent;
class EntityEventManager;

	}

	namespace physics
	{

class Body;
struct CollisionInfo;

/*! \brief
 * \ingroup Physics
 */
class T_DLLCLASS RigidBodyComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	RigidBodyComponent(
		Body* body,
		world::EntityEventManager* eventManager,
		world::IEntityEvent* eventCollide
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	Body* getBody() const { return m_body; }

private:
	world::Entity* m_owner;
	Ref< Body > m_body;
	Ref< world::EntityEventManager > m_eventManager;
	Ref< world::IEntityEvent > m_eventCollide;

	void collisionListener(const physics::CollisionInfo& collisionInfo);
};

	}
}

