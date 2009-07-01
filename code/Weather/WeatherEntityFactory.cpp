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

WeatherEntityFactory::WeatherEntityFactory(resource::IResourceManager* resourceManager, render::RenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
{
}

const TypeSet WeatherEntityFactory::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< SkyEntityData >());
	typeSet.insert(&type_of< CloudEntityData >());
	return typeSet;
}

world::Entity* WeatherEntityFactory::createEntity(world::IEntityBuilder* builder, const std::wstring& name, const world::EntityData& entityData) const
{
	if (const SkyEntityData* skyEntityData = dynamic_type_cast< const SkyEntityData* >(&entityData))
		return skyEntityData->createEntity(m_resourceManager, m_renderSystem);
	if (const CloudEntityData* cloudEntityData = dynamic_type_cast< const CloudEntityData* >(&entityData))
		return cloudEntityData->createEntity(m_resourceManager, m_renderSystem);
	return 0;
}

	}
}
