#include "World/IEntityRenderer.h"
#include "World/Renderable.h"
#include "World/WorldSetupContext.h"
#include "World/WorldEntityRenderers.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldSetupContext", WorldSetupContext, Object)

WorldSetupContext::WorldSetupContext(const WorldEntityRenderers* entityRenderers, const Entity* rootEntity, render::RenderGraph& renderGraph)
:	m_entityRenderers(entityRenderers)
,	m_rootEntity(rootEntity)
,	m_renderGraph(renderGraph)
{
}

void WorldSetupContext::setup(const WorldRenderView& worldRenderView, const Renderable* renderable) const
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
		renderer->setup(*this, worldRenderView, const_cast< Renderable* >(renderable));
}

void WorldSetupContext::flush() const
{
	for (auto entityRenderer : m_entityRenderers->get())
		entityRenderer->setup(*this);
}

	}
}
