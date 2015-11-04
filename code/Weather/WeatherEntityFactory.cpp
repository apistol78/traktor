#include "Weather/WeatherEntityFactory.h"
#include "Weather/Clouds/CloudEntityData.h"
#include "Weather/Clouds/CloudEntity.h"
#include "Weather/Sky/SkyComponent.h"
#include "Weather/Sky/SkyComponentData.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.WeatherEntityFactory", WeatherEntityFactory, world::IEntityFactory)

WeatherEntityFactory::WeatherEntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet WeatherEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< CloudEntityData >());
	return typeSet;
}

const TypeInfoSet WeatherEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet WeatherEntityFactory::getEntityComponentTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SkyComponentData >());
	return typeSet;
}

Ref< world::Entity > WeatherEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	if (const CloudEntityData* cloudEntityData = dynamic_type_cast< const CloudEntityData* >(&entityData))
		return cloudEntityData->createEntity(m_resourceManager, m_renderSystem);
	else
		return 0;
}

Ref< world::IEntityEvent > WeatherEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return 0;
}

Ref< world::IEntityComponent > WeatherEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, world::Entity* owner, const world::IEntityComponentData& entityComponentData) const
{
	if (const SkyComponentData* skyComponentData = dynamic_type_cast< const SkyComponentData* >(&entityComponentData))
		return skyComponentData->createComponent(m_resourceManager, m_renderSystem);
	else
		return 0;
}

	}
}
