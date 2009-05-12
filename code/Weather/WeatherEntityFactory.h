#ifndef traktor_weather_WeatherEntityFactory_H
#define traktor_weather_WeatherEntityFactory_H

#include "Core/Heap/Ref.h"
#include "World/Entity/EntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class RenderSystem;

	}

	namespace weather
	{

class T_DLLCLASS WeatherEntityFactory : public world::EntityFactory
{
	T_RTTI_CLASS(WeatherEntityFactory)

public:
	WeatherEntityFactory(render::RenderSystem* renderSystem);

	virtual const TypeSet getEntityTypes() const;

	virtual world::Entity* createEntity(world::EntityBuilder* builder, const world::EntityData& entityData) const;

private:
	Ref< render::RenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_weather_WeatherEntityFactory_H