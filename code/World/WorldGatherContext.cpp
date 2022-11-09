#include "World/IEntityRenderer.h"
#include "World/WorldGatherContext.h"
#include "World/WorldEntityRenderers.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldGatherContext", WorldGatherContext, Object)

WorldGatherContext::WorldGatherContext(const WorldEntityRenderers* entityRenderers, const Entity* rootEntity, const gatherFn_t& filter)
:	m_entityRenderers(entityRenderers)
,	m_rootEntity(rootEntity)
,	m_filter(filter)
{
}

void WorldGatherContext::gather(Object* renderable) const
{
	if (!renderable)
		return;

	IEntityRenderer* entityRenderer = m_entityRenderers->find(type_of(renderable));
	if (!entityRenderer)
		return;

	entityRenderer->gather(*this, renderable);
}

void WorldGatherContext::include(IEntityRenderer* entityRenderer, Object* renderable) const
{
	m_filter(entityRenderer, renderable);
}

}
