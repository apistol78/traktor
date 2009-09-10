#include "World/WorldContext.h"
#include "World/WorldRenderer.h"
#include "World/WorldEntityRenderers.h"
#include "World/Entity/Entity.h"
#include "World/Entity/IEntityRenderer.h"
#include "Render/Context/RenderContext.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldContext", WorldContext, Object)

WorldContext::WorldContext(WorldRenderer* worldRenderer, WorldEntityRenderers* entityRenderers, render::IRenderView* renderView)
:	m_worldRenderer(worldRenderer)
,	m_entityRenderers(entityRenderers)
,	m_renderContext(gc_new< render::RenderContext >(renderView))
,	m_lastEntityType(0)
{
}

void WorldContext::build(WorldRenderView* worldRenderView, Entity* entity)
{
	if (!entity)
		return;

	IEntityRenderer* entityRenderer = 0;

	const Type& entityType = entity->getType();
	if (m_lastEntityType == &entityType)
	{
		// Fast path, no need to lookup new entity renderer as it's the same as last entity rendered.
		entityRenderer = m_lastEntityRenderer;
	}
	else
	{
		// Need to find entity renderer.
		entityRenderer = m_entityRenderers->find(entityType);

		m_lastEntityType = &entityType;
		m_lastEntityRenderer = entityRenderer;
	}

	if (entityRenderer)
		entityRenderer->render(this, worldRenderView, entity);
}

void WorldContext::flush(WorldRenderView* worldRenderView)
{
	T_ASSERT (m_entityRenderers);
	const RefArray< IEntityRenderer >& entityRenderers = m_entityRenderers->get();
	for (RefArray< IEntityRenderer >::const_iterator i = entityRenderers.begin(); i != entityRenderers.end(); ++i)
		(*i)->flush(this, worldRenderView);
}

	}
}
