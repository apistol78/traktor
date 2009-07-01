#include "Physics/World/EntityFactory.h"
#include "Physics/World/RigidEntityData.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/World/ArticulatedEntityData.h"
#include "Physics/World/ArticulatedEntity.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.EntityFactory", EntityFactory, world::IEntityFactory)

EntityFactory::EntityFactory(physics::PhysicsManager* physicsManager)
:	m_physicsManager(physicsManager)
{
}

const TypeSet EntityFactory::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< RigidEntityData >());
	typeSet.insert(&type_of< ArticulatedEntityData >());
	return typeSet;
}

world::Entity* EntityFactory::createEntity(
	world::IEntityBuilder* builder,
	const std::wstring& name,
	const world::EntityData& entityData
) const
{
	if (const RigidEntityData* rigidEntityData = dynamic_type_cast< const RigidEntityData* >(&entityData))
		return rigidEntityData->createEntity(builder, m_physicsManager);

	if (const ArticulatedEntityData* articulatedEntityData = dynamic_type_cast< const ArticulatedEntityData* >(&entityData))
		return articulatedEntityData->createEntity(builder, m_physicsManager);

	return 0;
}

	}
}
