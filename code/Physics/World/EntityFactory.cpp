/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/World/EntityFactory.h"
#include "Physics/World/JointComponent.h"
#include "Physics/World/JointComponentData.h"
#include "Physics/World/RigidBodyComponent.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Physics/World/Character/CharacterComponent.h"
#include "Physics/World/Character/CharacterComponentData.h"
#include "Physics/World/Vehicle/VehicleComponent.h"
#include "Physics/World/Vehicle/VehicleComponentData.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.EntityFactory", EntityFactory, world::IEntityFactory)

EntityFactory::EntityFactory(
	resource::IResourceManager* resourceManager,
	PhysicsManager* physicsManager
)
:	m_resourceManager(resourceManager)
,	m_physicsManager(physicsManager)
{
}

const TypeInfoSet EntityFactory::getEntityTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet EntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet EntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet<
		CharacterComponentData,
		JointComponentData,
		RigidBodyComponentData,
		VehicleComponentData
	>();
}

Ref< world::Entity > EntityFactory::createEntity(
	const world::IEntityBuilder* builder,
	const world::EntityData& entityData
) const
{
	return nullptr;
}

Ref< world::IEntityEvent > EntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > EntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (auto characterComponentData = dynamic_type_cast< const CharacterComponentData* >(&entityComponentData))
		return characterComponentData->createComponent(builder, m_resourceManager, m_physicsManager);
	else if (auto jointComponentData = dynamic_type_cast< const JointComponentData* >(&entityComponentData))
		return jointComponentData->createComponent(m_physicsManager);
	else if (auto rigidBodyComponentData = dynamic_type_cast< const RigidBodyComponentData* >(&entityComponentData))
		return rigidBodyComponentData->createComponent(builder, m_resourceManager, m_physicsManager);
	else if (auto vehicleComponentData = dynamic_type_cast< const VehicleComponentData* >(&entityComponentData))
		return vehicleComponentData->createComponent(builder, m_resourceManager, m_physicsManager);
	else
		return nullptr;
}

}
