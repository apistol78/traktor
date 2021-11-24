#pragma once

#include <functional>
#include "Core/Ref.h"
#include "World/IEntityRenderer.h"

namespace traktor
{
	namespace scene
	{

class EntityRendererCache;

class EntityRendererAdapter : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	explicit EntityRendererAdapter(EntityRendererCache* cache, world::IEntityRenderer* entityRenderer, const std::function< bool(const EntityAdapter*) >& filter);

	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void gather(
		const world::WorldGatherContext& context,
		Object* renderable
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
	Ref< EntityRendererCache > m_cache;
	Ref< world::IEntityRenderer > m_entityRenderer;
	std::function< bool(const EntityAdapter*) > m_filter;
};

	}
}

