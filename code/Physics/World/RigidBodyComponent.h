/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_RigidBodyComponent_H
#define traktor_physics_RigidBodyComponent_H

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
class IEntityEventManager;

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
		world::IEntityEventManager* eventManager,
		world::IEntityEvent* eventCollide
	);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setOwner(world::Entity* owner) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

private:
	world::Entity* m_owner;
	Ref< Body > m_body;
	Ref< world::IEntityEventManager > m_eventManager;
	Ref< world::IEntityEvent > m_eventCollide;

	void collisionListener(const physics::CollisionInfo& collisionInfo);
};

	}
}

#endif	// traktor_physics_RigidBodyComponent_H
