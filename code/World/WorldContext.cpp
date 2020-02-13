#include "World/IEntityRenderer.h"
#include "World/WorldContext.h"
#include "World/WorldEntityRenderers.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldContext", WorldContext, Object)

WorldContext::WorldContext(WorldEntityRenderers* entityRenderers, Entity* rootEntity)
:	m_entityRenderers(entityRenderers)
,	m_renderContext(nullptr)
,	m_rootEntity(rootEntity)
{
}

WorldContext::WorldContext(WorldEntityRenderers* entityRenderers, render::RenderContext* renderContext, Entity* rootEntity)
:	m_entityRenderers(entityRenderers)
,	m_renderContext(renderContext)
,	m_rootEntity(rootEntity)
{
}

void WorldContext::gather(const Object* renderable, AlignedVector< Light >& outLights) const
{
	if (!renderable)
		return;
	IEntityRenderer* renderer = m_entityRenderers->find(type_of(renderable));
	if (renderer)
		renderer->gather(*this, renderable, outLights);
}

void WorldContext::build(const WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass, Object* renderable) const
{
	if (!renderable)
		return;
	IEntityRenderer* renderer = m_entityRenderers->find(type_of(renderable));
	if (renderer)
		renderer->build(*this, worldRenderView, worldRenderPass, renderable);
}

void WorldContext::flush(const WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass) const
{
	for (auto entityRenderer : m_entityRenderers->get())
		entityRenderer->flush(*this, worldRenderView, worldRenderPass);
}

void WorldContext::flush() const
{
	for (auto entityRenderer : m_entityRenderers->get())
		entityRenderer->flush(*this);
}

	}
}
