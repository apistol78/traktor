#include "Render/Context/RenderContext.h"
#include "World/Entity.h"
#include "World/IEntityRenderer.h"
#include "World/IWorldCulling.h"
#include "World/WorldContext.h"
#include "World/WorldEntityRenderers.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const uint32_t c_renderContextSize = 2 * 1024 * 1024;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldContext", WorldContext, Object)

WorldContext::WorldContext(
	WorldEntityRenderers* entityRenderers,
	IWorldCulling* culling
)
:	m_entityRenderers(entityRenderers)
,	m_culling(culling)
,	m_renderContext(new render::RenderContext(c_renderContextSize))
,	m_lastEntityType(0)
,	m_lastEntityRenderer(0)
{
}

void WorldContext::clear()
{
	if (m_renderContext)
		m_renderContext->flush();
}

void WorldContext::precull(WorldRenderView& worldRenderView, Entity* entity)
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
		entityRenderer->precull(*this, worldRenderView, entity);
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
