#ifndef traktor_weather_WeatherEntityRenderer_H
#define traktor_weather_WeatherEntityRenderer_H

#include "World/IEntityRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class PrimitiveRenderer;

	}

	namespace weather
	{

class T_DLLCLASS WeatherEntityRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	WeatherEntityRenderer(render::PrimitiveRenderer* primitiveRenderer = 0);

	virtual const TypeInfoSet getRenderableTypes() const T_OVERRIDE T_FINAL;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		Object* renderable
	) T_OVERRIDE T_FINAL;

	virtual void flush(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	) T_OVERRIDE T_FINAL;

private:
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
};

	}
}

#endif	// traktor_weather_WeatherEntityRenderer_H
