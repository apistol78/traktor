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
	const world::WorldContext& worldContext,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
	m_entityRenderer->gather(worldContext, renderable, outLights);
}

void EntityRendererAdapter::build(
	const world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	EntityAdapter* entityAdapter = m_cache->begin(renderable);
	if (!entityAdapter || m_filter(entityAdapter))
	{
		m_entityRenderer->build(
			worldContext,
			worldRenderView,
			worldRenderPass,
			renderable
		);
	}
	m_cache->end();
}

void EntityRendererAdapter::flush(
	const world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	m_entityRenderer->flush(
		worldContext,
		worldRenderView,
		worldRenderPass
	);
}

void EntityRendererAdapter::flush(const world::WorldContext& worldContext)
{
	m_entityRenderer->flush(worldContext);
}

	}
}
