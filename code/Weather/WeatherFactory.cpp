#include "Weather/WeatherFactory.h"
#include "Weather/Clouds/CloudComponent.h"
#include "Weather/Clouds/CloudComponentData.h"
#include "Weather/Sky/SkyComponent.h"
#include "Weather/Sky/SkyComponentData.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.WeatherFactory", WeatherFactory, world::IEntityFactory)

WeatherFactory::WeatherFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet WeatherFactory::getEntityTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet WeatherFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet WeatherFactory::getEntityComponentTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< CloudComponentData >());
	typeSet.insert(&type_of< SkyComponentData >());
	return typeSet;
}

Ref< world::Entity > WeatherFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return 0;
}

Ref< world::IEntityEvent > WeatherFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return 0;
}

Ref< world::IEntityComponent > WeatherFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (const CloudComponentData* cloudComponentData = dynamic_type_cast< const CloudComponentData* >(&entityComponentData))
		return cloudComponentData->createComponent(m_resourceManager, m_renderSystem);
	else if (const SkyComponentData* skyComponentData = dynamic_type_cast< const SkyComponentData* >(&entityComponentData))
		return skyComponentData->createComponent(m_resourceManager, m_renderSystem);
	else
		return 0;
}

	}
}
