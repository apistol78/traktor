#include "World/IEntityRenderer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldBuildContext", WorldBuildContext, Object)

WorldBuildContext::WorldBuildContext(const WorldEntityRenderers* entityRenderers, const Entity* rootEntity, render::RenderContext* renderContext)
:	m_entityRenderers(entityRenderers)
,	m_renderContext(renderContext)
,	m_rootEntity(rootEntity)
{
}

}
