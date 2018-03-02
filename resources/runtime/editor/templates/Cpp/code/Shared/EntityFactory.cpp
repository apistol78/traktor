#include "Shared/EntityFactory.h"
#include "Shared/QuadComponent.h"
#include "Shared/QuadComponentData.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"EntityFactory", EntityFactory, world::IEntityFactory)

EntityFactory::EntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
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
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< QuadComponentData >());
	return typeSet;
}

Ref< world::Entity > EntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return 0;
}

Ref< world::IEntityEvent > EntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return 0;
}

Ref< world::IEntityComponent > EntityFactory::createEntityComponent(const world::IEntityBuilder* builder, world::Entity* owner, const world::IEntityComponentData& entityComponentData) const
{
	const QuadComponentData* quadComponentData = mandatory_non_null_type_cast< const QuadComponentData* >(&entityComponentData);
	return quadComponentData->createComponent(m_resourceManager, m_renderSystem);
}
