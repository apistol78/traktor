/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/ArticulatedEntityData.h"
#include "Physics/World/EntityFactory.h"
#include "Physics/World/RigidBodyComponent.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/World/RigidEntityData.h"
#include "Physics/World/Character/CharacterComponent.h"
#include "Physics/World/Character/CharacterComponentData.h"
#include "Physics/World/Vehicle/VehicleComponent.h"
#include "Physics/World/Vehicle/VehicleComponentData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.EntityFactory", EntityFactory, world::IEntityFactory)

EntityFactory::EntityFactory(
	world::IEntityEventManager* eventManager,
	resource::IResourceManager* resourceManager,
	PhysicsManager* physicsManager
)
:	m_eventManager(eventManager)
,	m_resourceManager(resourceManager)
,	m_physicsManager(physicsManager)
{
}

const TypeInfoSet EntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ArticulatedEntityData >());
	typeSet.insert(&type_of< RigidEntityData >());
	return typeSet;
}

const TypeInfoSet EntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet EntityFactory::getEntityComponentTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< CharacterComponentData >());
	typeSet.insert(&type_of< RigidBodyComponentData >());
	typeSet.insert(&type_of< VehicleComponentData >());
	return typeSet;
}

Ref< world::Entity > EntityFactory::createEntity(
	const world::IEntityBuilder* builder,
	const world::EntityData& entityData
) const
{
	if (const ArticulatedEntityData* articulatedEntityData = dynamic_type_cast< const ArticulatedEntityData* >(&entityData))
		return articulatedEntityData->createEntity(builder, m_physicsManager);

	if (const RigidEntityData* rigidEntityData = dynamic_type_cast< const RigidEntityData* >(&entityData))
		return rigidEntityData->createEntity(builder, m_eventManager, m_resourceManager, m_physicsManager);

	return 0;
}

Ref< world::IEntityEvent > EntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return 0;
}

Ref< world::IEntityComponent > EntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (const CharacterComponentData* characterComponentData = dynamic_type_cast< const CharacterComponentData* >(&entityComponentData))
		return characterComponentData->createComponent(builder, m_resourceManager, m_physicsManager);

	if (const RigidBodyComponentData* rigidBodyComponentData = dynamic_type_cast< const RigidBodyComponentData* >(&entityComponentData))
		return rigidBodyComponentData->createComponent(builder, m_eventManager, m_resourceManager, m_physicsManager);

	if (const VehicleComponentData* vehicleComponentData = dynamic_type_cast< const VehicleComponentData* >(&entityComponentData))
		return vehicleComponentData->createComponent(builder, m_resourceManager, m_physicsManager);

	return 0;
}

	}
}
