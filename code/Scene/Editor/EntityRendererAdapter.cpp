#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityRendererAdapter.h"
#include "Scene/Editor/EntityRendererCache.h"

namespace traktor
{
	namespace scene
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityRendererAdapter", EntityRendererAdapter, world::IEntityRenderer)

EntityRendererAdapter::EntityRendererAdapter(EntityRendererCache* cache, world::IEntityRenderer* entityRenderer)
:	m_cache(cache)
,	m_entityRenderer(entityRenderer)
{
}

const TypeInfoSet EntityRendererAdapter::getEntityTypes() const
{
	return m_entityRenderer->getEntityTypes();
}

void EntityRendererAdapter::precull(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::Entity* entity
)
{
	EntityAdapter* entityAdapter = m_cache->begin(entity);
	//if (!entityAdapter || entityAdapter->isVisible())
	{
		m_entityRenderer->precull(
			worldContext,
			worldRenderView,
			entity
		);
	}
	m_cache->end();
}

void EntityRendererAdapter::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	world::Entity* entity
)
{
	EntityAdapter* entityAdapter = m_cache->begin(entity);
	if (!entityAdapter || entityAdapter->isVisible())
	{
		m_entityRenderer->render(
			worldContext,
			worldRenderView,
			worldRenderPass,
			entity
		);
	}
	m_cache->end();
}

void EntityRendererAdapter::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	m_entityRenderer->flush(
		worldContext,
		worldRenderView,
		worldRenderPass
	);
}
	
	}
}
