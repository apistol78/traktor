#include "World/IEntityRenderer.h"
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

void WorldBuildContext::build(const WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass, const Object* renderable) const
{
	if (!renderable)
		return;
	IEntityRenderer* renderer = m_entityRenderers->find(type_of(renderable));
	if (renderer)
		renderer->build(*this, worldRenderView, worldRenderPass, const_cast< Object* >(renderable));
}

void WorldBuildContext::flush(const WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass) const
{
	for (auto entityRenderer : m_entityRenderers->get())
		entityRenderer->build(*this, worldRenderView, worldRenderPass);
}

	}
}
