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
{
}

void WorldContext::build(WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass, Object* renderable)
{
	if (!renderable)
		return;
	IEntityRenderer* renderer = m_entityRenderers->find(type_of(renderable));
	if (renderer)
		renderer->render(*this, worldRenderView, worldRenderPass, renderable);
}

void WorldContext::flush(WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass, Entity* rootEntity)
{
	for (auto entityRenderer : m_entityRenderers->get())
		entityRenderer->flush(*this, worldRenderView, worldRenderPass, rootEntity);
}

void WorldContext::flush(Entity* rootEntity)
{
	for (auto entityRenderer : m_entityRenderers->get())
		entityRenderer->flush(*this, rootEntity);
}

	}
}
