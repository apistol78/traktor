#include "Render/Context/RenderContext.h"
#include "World/WorldContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/Entity/Entity.h"
#include "World/Entity/IEntityRenderer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldContext", WorldContext, Object)

WorldContext::WorldContext(WorldEntityRenderers* entityRenderers)
:	m_entityRenderers(entityRenderers)
,	m_renderContext(new render::RenderContext())
,	m_lastEntityType(0)
,	m_lastEntityRenderer(0)
{
}

void WorldContext::build(WorldRenderView& worldRenderView, IWorldRenderPass& worldRenderPass, Entity* entity)
{
	if (!entity)
		return;

	IEntityRenderer* entityRenderer = 0;

	const TypeInfo& entityType = type_of(entity);
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
		entityRenderer->render(*this, worldRenderView, worldRenderPass, entity);
}

void WorldContext::flush(WorldRenderView& worldRenderView, IWorldRenderPass& worldRenderPass)
{
	T_ASSERT (m_entityRenderers);
	const RefArray< IEntityRenderer >& entityRenderers = m_entityRenderers->get();
	for (RefArray< IEntityRenderer >::const_iterator i = entityRenderers.begin(); i != entityRenderers.end(); ++i)
		(*i)->flush(*this, worldRenderView, worldRenderPass);
}

	}
}
