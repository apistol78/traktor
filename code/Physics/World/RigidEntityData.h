/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_RigidEntityData_H
#define traktor_physics_RigidEntityData_H

#include "World/EntityData.h"

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

class IEntityBuilder;
class IEntityEventData;
class IEntityEventManager;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace physics
	{

class BodyDesc;
class PhysicsManager;
class RigidEntity;

/*! \brief Rigid body entity data.
 * \ingroup Physics
 */
class T_DLLCLASS RigidEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	Ref< RigidEntity > createEntity(
		const world::IEntityBuilder* entityBuilder,
		world::IEntityEventManager* eventManager,
		resource::IResourceManager* resourceManager,
		PhysicsManager* physicsManager
	) const;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	void setBodyDesc(BodyDesc* bodyDesc) { m_bodyDesc = bodyDesc; }

	const BodyDesc* getBodyDesc() const { return m_bodyDesc; }

	void setEntityData(world::EntityData* entityData) { m_entityData = entityData; }

	const world::EntityData* getEntityData() const { return m_entityData; }

private:
	Ref< BodyDesc > m_bodyDesc;
	Ref< world::EntityData > m_entityData;
	Ref< world::IEntityEventData > m_eventCollide;
};

	}
}

#endif	// traktor_physics_RigidEntityData_H
