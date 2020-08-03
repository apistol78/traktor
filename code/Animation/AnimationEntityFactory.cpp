#include "Animation/AnimatedMeshComponent.h"
#include "Animation/AnimatedMeshComponentData.h"
#include "Animation/AnimationEntityFactory.h"
#include "Animation/Boids/BoidsComponent.h"
#include "Animation/Boids/BoidsComponentData.h"
#include "Animation/Cloth/ClothComponent.h"
#include "Animation/Cloth/ClothComponentData.h"
#include "Animation/PathEntity/PathComponentData.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationEntityFactory", AnimationEntityFactory, world::IEntityFactory)

AnimationEntityFactory::AnimationEntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, physics::PhysicsManager* physicsManager)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_physicsManager(physicsManager)
{
}

const TypeInfoSet AnimationEntityFactory::getEntityTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet AnimationEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet AnimationEntityFactory::getEntityComponentTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< AnimatedMeshComponentData >();
	typeSet.insert< BoidsComponentData >();
	typeSet.insert< ClothComponentData >();
	typeSet.insert< PathComponentData >();
	return typeSet;
}

Ref< world::Entity > AnimationEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return nullptr;
}

Ref< world::IEntityEvent > AnimationEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > AnimationEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (auto animatedMeshComponentData = dynamic_type_cast< const AnimatedMeshComponentData* >(&entityComponentData))
		return animatedMeshComponentData->createComponent(m_resourceManager, m_physicsManager, builder);
	else if (auto boidsComponentData = dynamic_type_cast< const BoidsComponentData* >(&entityComponentData))
		return boidsComponentData->createComponent(builder);
	else if (auto clothComponentData = dynamic_type_cast< const ClothComponentData* >(&entityComponentData))
		return clothComponentData->createComponent(m_resourceManager, m_renderSystem);
	else if (auto pathComponentData = dynamic_type_cast< const PathComponentData* >(&entityComponentData))
		return pathComponentData->createComponent();
	else
		return nullptr;
}

	}
}
