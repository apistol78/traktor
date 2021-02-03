#include "World/IEntityRenderer.h"
#include "World/WorldGatherContext.h"
#include "World/WorldEntityRenderers.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldGatherContext", WorldGatherContext, Object)

WorldGatherContext::WorldGatherContext(const WorldEntityRenderers* entityRenderers, const Entity* rootEntity)
:	m_entityRenderers(entityRenderers)
,	m_rootEntity(rootEntity)
{
}

void WorldGatherContext::gather(const Object* renderable, AlignedVector< const LightComponent* >& outLights, AlignedVector< const ProbeComponent* >& outProbes) const
{
	if (!renderable)
		return;
	IEntityRenderer* renderer = m_entityRenderers->find(type_of(renderable));
	if (renderer)
		renderer->gather(*this, renderable, outLights, outProbes);
}

	}
}
