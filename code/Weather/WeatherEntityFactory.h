#ifndef traktor_weather_WeatherEntityFactory_H
#define traktor_weather_WeatherEntityFactory_H

#include "Core/Heap/Ref.h"
#include "World/Entity/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS(WeatherEntityFactory)

public:
	WeatherEntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	virtual const TypeSet getEntityTypes() const;

	virtual world::Entity* createEntity(world::IEntityBuilder* builder, const std::wstring& name, const world::EntityData& entityData, const Object* instanceData) const;

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_weather_WeatherEntityFactory_H