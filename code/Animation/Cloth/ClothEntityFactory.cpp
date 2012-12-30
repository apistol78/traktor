#include "Animation/Cloth/ClothEntity.h"
#include "Animation/Cloth/ClothEntityData.h"
#include "Animation/Cloth/ClothEntityFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.ClothEntityFactory", ClothEntityFactory, world::IEntityFactory)

ClothEntityFactory::ClothEntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet ClothEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ClothEntityData >());
	return typeSet;
}

Ref< world::Entity > ClothEntityFactory::createEntity(world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return checked_type_cast< const ClothEntityData* >(&entityData)->createEntity(builder, m_resourceManager, m_renderSystem);
}

	}
}
