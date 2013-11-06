#include "Animation/AnimatedMeshEntity.h"
#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/AnimatedMeshEntityFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshEntityFactory", AnimatedMeshEntityFactory, world::IEntityFactory)

AnimatedMeshEntityFactory::AnimatedMeshEntityFactory(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager)
:	m_resourceManager(resourceManager)
,	m_physicsManager(physicsManager)
{
}

const TypeInfoSet AnimatedMeshEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< AnimatedMeshEntityData >());
	return typeSet;
}

const TypeInfoSet AnimatedMeshEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

Ref< world::Entity > AnimatedMeshEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return checked_type_cast< const AnimatedMeshEntityData* >(&entityData)->createEntity(m_resourceManager, m_physicsManager, builder);
}

Ref< world::IEntityEvent > AnimatedMeshEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return 0;
}

	}
}
