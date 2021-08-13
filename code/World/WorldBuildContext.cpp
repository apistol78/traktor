#include "World/IEntityRenderer.h"
#include "World/Renderable.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldBuildContext", WorldBuildContext, Object)

WorldBuildContext::WorldBuildContext(const WorldEntityRenderers* entityRenderers, const Entity* rootEntity, render::RenderContext* renderContext)
:	m_entityRenderers(entityRenderers)
,	m_renderContext(renderContext)
,	m_rootEntity(rootEntity)
{
}

void WorldBuildContext::build(const WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass, const Renderable* renderable) const
{
	if (!renderable)
		return;

	IEntityRenderer* renderer = nullptr;

	if (renderable->m_entityRenderers == m_entityRenderers)
		renderer = renderable->m_entityRenderer;
	else
	{
		renderer = m_entityRenderers->find(type_of(renderable));
		renderable->m_entityRenderers = m_entityRenderers;
		renderable->m_entityRenderer = renderer;
	}

	if (renderer)
		renderer->build(*this, worldRenderView, worldRenderPass, const_cast< Renderable* >(renderable));
}

void WorldBuildContext::flush(const WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass) const
{
	for (auto renderer : m_entityRenderers->get())
		renderer->build(*this, worldRenderView, worldRenderPass);
}

	}
}
