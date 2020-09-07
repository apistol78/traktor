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

class T_DLLCLASS CloudRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	CloudRenderer(render::PrimitiveRenderer* primitiveRenderer = nullptr);

	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void gather(
		const world::WorldGatherContext& context,
		const Object* renderable,
		AlignedVector< world::Light >& outLights
	) override final;

	virtual void setup(
		const world::WorldSetupContext& context,
		const world::WorldRenderView& worldRenderView,
		Object* renderable
	) override final;

	virtual void setup(
		const world::WorldSetupContext& context
	) override final;

	virtual void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	) override final;

private:
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
};

	}
}

