#include "World/Entity.h"
#include "World/WorldBuildContext.h"
#include "World/WorldGatherContext.h"
#include "World/WorldSetupContext.h"
#include "World/Entity/FacadeComponent.h"
#include "World/Entity/FacadeRenderer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.FacadeRenderer", FacadeRenderer, IEntityRenderer)

FacadeRenderer::FacadeRenderer()
{
}

const TypeInfoSet FacadeRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< FacadeComponent >();
}

void FacadeRenderer::gather(
	const WorldGatherContext& context,
	Object* renderable
)
{
	auto facadeComponent = static_cast< const FacadeComponent* >(renderable);
	for (auto childEntity : facadeComponent->getVisibleEntities())
		context.gather(childEntity);
}

void FacadeRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void FacadeRenderer::setup(
	const WorldSetupContext& context
)
{
}

void FacadeRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
}

void FacadeRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

	}
}
