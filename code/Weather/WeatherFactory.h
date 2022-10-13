#pragma once

#include "World/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class IRenderSystem;

}

namespace traktor::weather
{

/*! Weather entity factory.
 * \ingroup Weather
 */
class T_DLLCLASS WeatherFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;

public:
	explicit WeatherFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	virtual const TypeInfoSet getEntityTypes() const override final;

	virtual const TypeInfoSet getEntityEventTypes() const override final;

	virtual const TypeInfoSet getEntityComponentTypes() const override final;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const override final;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const override final;

	virtual Ref< world::IEntityComponent > createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const override final;

private:
	resource::IResourceManager* m_resourceManager;
	render::IRenderSystem* m_renderSystem;
};

}
