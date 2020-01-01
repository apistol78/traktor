#include "Render/Context/RenderContext.h"
#include "World/IEntityRenderer.h"
#include "World/WorldContext.h"
#include "World/WorldEntityRenderers.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldContext", WorldContext, Object)

WorldContext::WorldContext(WorldEntityRenderers* entityRenderers, render::RenderContext* renderContext)
:	m_entityRenderers(entityRenderers)
,	m_renderContext(renderContext)
,	m_lastRenderableType(nullptr)
,	m_lastRenderer(nullptr)
{
}

void WorldContext::build(WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass, Object* renderable)
{
	if (!renderable)
		return;

	IEntityRenderer* renderer = nullptr;

	const TypeInfo& renderableType = type_of(renderable);
	if (m_lastRenderableType == &renderableType)
	{
		// Fast path, no need to lookup new entity renderer as it's the same as last entity rendered.
		renderer = m_lastRenderer;
	}
	else
	{
		// Need to find entity renderer.
		renderer = m_entityRenderers->find(renderableType);
		m_lastRenderableType = &renderableType;
		m_lastRenderer = renderer;
	}

	if (renderer)
		renderer->render(*this, worldRenderView, worldRenderPass, renderable);
}

void WorldContext::flush(WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass, Entity* rootEntity)
{
	for (auto entityRenderer : m_entityRenderers->get())
		entityRenderer->flush(*this, worldRenderView, worldRenderPass, rootEntity);
}

	}
}
