#include "Weather/WeatherEntityFactory.h"
#include "Weather/Sky/SkyEntityData.h"
#include "Weather/Sky/SkyEntity.h"
#include "Weather/Clouds/CloudEntityData.h"
#include "Weather/Clouds/CloudEntity.h"

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
	typeSet.insert(&type_of< SkyEntityData >());
	typeSet.insert(&type_of< CloudEntityData >());
	return typeSet;
}

const TypeInfoSet WeatherEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet WeatherEntityFactory::getEntityComponentTypes() const
{
	return TypeInfoSet();
}

Ref< world::Entity > WeatherEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	if (const SkyEntityData* skyEntityData = dynamic_type_cast< const SkyEntityData* >(&entityData))
		return skyEntityData->createEntity(m_resourceManager, m_renderSystem);
	else if (const CloudEntityData* cloudEntityData = dynamic_type_cast< const CloudEntityData* >(&entityData))
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
	return 0;
}

	}
}
