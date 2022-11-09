#include "World/IEntityRenderer.h"
#include "World/WorldSetupContext.h"
#include "World/WorldEntityRenderers.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldSetupContext", WorldSetupContext, Object)

WorldSetupContext::WorldSetupContext(const WorldEntityRenderers* entityRenderers, const Entity* rootEntity, render::RenderGraph& renderGraph)
:	m_entityRenderers(entityRenderers)
,	m_rootEntity(rootEntity)
,	m_renderGraph(renderGraph)
{
}

}
