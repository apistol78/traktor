#pragma once

#include "Core/Ref.h"
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

class T_DLLCLASS WeatherRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	WeatherRenderer(render::PrimitiveRenderer* primitiveRenderer = 0);

	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void flush(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		world::Entity* rootEntity
	) override final;

private:
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
};

	}
}

