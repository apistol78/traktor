#include "Animation/Cloth/ClothComponent.h"
#include "Animation/Cloth/ClothComponentData.h"
#include "Animation/Cloth/ClothFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.ClothFactory", ClothFactory, world::IEntityFactory)

ClothFactory::ClothFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet ClothFactory::getEntityTypes() const
{
	return makeTypeInfoSet< ClothComponentData >();
}

const TypeInfoSet ClothFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet ClothFactory::getEntityComponentTypes() const
{
	return TypeInfoSet();
}

Ref< world::Entity > ClothFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return nullptr;
}

Ref< world::IEntityEvent > ClothFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > ClothFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	return checked_type_cast< const ClothComponentData* >(&entityComponentData)->createComponent(m_resourceManager, m_renderSystem);
}

	}
}
