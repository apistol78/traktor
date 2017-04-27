/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_RigidBodyComponentData_H
#define traktor_physics_RigidBodyComponentData_H

#include "World/IEntityComponentData.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class Entity;
class IEntityBuilder;
class IEntityEventData;
class IEntityEventManager;

	}

	namespace physics
	{

class BodyDesc;
class PhysicsManager;
class RigidBodyComponent;

/*! \brief
 * \ingroup Physics
 */
class T_DLLCLASS RigidBodyComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	RigidBodyComponentData();

	explicit RigidBodyComponentData(BodyDesc* bodyDesc);

	explicit RigidBodyComponentData(BodyDesc* bodyDesc, world::IEntityEventData* eventCollide);

	Ref< RigidBodyComponent > createComponent(
		const world::IEntityBuilder* entityBuilder,
		world::IEntityEventManager* eventManager,
		resource::IResourceManager* resourceManager,
		PhysicsManager* physicsManager
	) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const BodyDesc* getBodyDesc() const { return m_bodyDesc; }

	const world::IEntityEventData* getEventCollide() const { return m_eventCollide; }

private:
	Ref< BodyDesc > m_bodyDesc;
	Ref< world::IEntityEventData > m_eventCollide;
};

	}
}

#endif	// traktor_physics_RigidBodyComponentData_H
