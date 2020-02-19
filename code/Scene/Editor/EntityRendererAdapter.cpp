#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityRendererAdapter.h"
#include "Scene/Editor/EntityRendererCache.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityRendererAdapter", EntityRendererAdapter, world::IEntityRenderer)

EntityRendererAdapter::EntityRendererAdapter(EntityRendererCache* cache, world::IEntityRenderer* entityRenderer, const std::function< bool(const EntityAdapter*) >& filter)
:	m_cache(cache)
,	m_entityRenderer(entityRenderer)
,	m_filter(filter)
{
}

const TypeInfoSet EntityRendererAdapter::getRenderableTypes() const
{
	return m_entityRenderer->getRenderableTypes();
}

void EntityRendererAdapter::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
	m_entityRenderer->gather(context, renderable, outLights);
}

void EntityRendererAdapter::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	EntityAdapter* entityAdapter = m_cache->begin(renderable);
	if (!entityAdapter || m_filter(entityAdapter))
	{
		m_entityRenderer->build(
			context,
			worldRenderView,
			worldRenderPass,
			renderable
		);
	}
	m_cache->end();
}

void EntityRendererAdapter::flush(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	m_entityRenderer->flush(
		context,
		worldRenderView,
		worldRenderPass
	);
}

void EntityRendererAdapter::setup(const world::WorldSetupContext& context)
{
	m_entityRenderer->setup(context);
}

	}
}
