#ifndef traktor_weather_WeatherEntityRenderer_H
#define traktor_weather_WeatherEntityRenderer_H

#include "Core/Heap/Ref.h"
#include "World/Entity/EntityRenderer.h"

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

class PrimitiveRenderer;

	}

	namespace weather
	{

class T_DLLCLASS WeatherEntityRenderer : public world::EntityRenderer
{
	T_RTTI_CLASS(WeatherEntityRenderer)

public:
	WeatherEntityRenderer(render::PrimitiveRenderer* primitiveRenderer = 0);

	virtual const TypeSet getEntityTypes() const;

	virtual void render(
		world::WorldContext* worldContext,
		world::WorldRenderView* worldRenderView,
		world::Entity* entity
	);

	virtual void flush(
		world::WorldContext* worldContext,
		world::WorldRenderView* worldRenderView
	);

private:
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
};

	}
}

#endif	// traktor_weather_WeatherEntityRenderer_H
