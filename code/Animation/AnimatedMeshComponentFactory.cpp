#include "Animation/AnimatedMeshComponent.h"
#include "Animation/AnimatedMeshComponentData.h"
#include "Animation/AnimatedMeshComponentFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshComponentFactory", AnimatedMeshComponentFactory, world::IEntityFactory)

AnimatedMeshComponentFactory::AnimatedMeshComponentFactory(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager)
:	m_resourceManager(resourceManager)
,	m_physicsManager(physicsManager)
{
}

const TypeInfoSet AnimatedMeshComponentFactory::getEntityTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet AnimatedMeshComponentFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet AnimatedMeshComponentFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet< AnimatedMeshComponentData >();
}

Ref< world::Entity > AnimatedMeshComponentFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return nullptr;
}

Ref< world::IEntityEvent > AnimatedMeshComponentFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > AnimatedMeshComponentFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	return mandatory_non_null_type_cast< const AnimatedMeshComponentData* >(&entityComponentData)->createComponent(m_resourceManager, m_physicsManager, builder);
}

	}
}
