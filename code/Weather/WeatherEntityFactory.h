#ifndef traktor_weather_WeatherEntityFactory_H
#define traktor_weather_WeatherEntityFactory_H

#include "World/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;

	}

	namespace weather
	{

class T_DLLCLASS WeatherEntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;

public:
	WeatherEntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual const TypeInfoSet getEntityEventTypes() const;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const;

private:
	resource::IResourceManager* m_resourceManager;
	render::IRenderSystem* m_renderSystem;
};

	}
}

#endif	// traktor_weather_WeatherEntityFactory_H