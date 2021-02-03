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
	return makeTypeInfoSet<
		FacadeComponent
	>();
}

void FacadeRenderer::gather(
	const WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< const LightComponent* >& outLights,
	AlignedVector< const ProbeComponent* >& outProbes
)
{
	auto facadeComponent = mandatory_non_null_type_cast< const FacadeComponent* >(renderable);
	for (auto childEntity : facadeComponent->getVisibleEntities())
		context.gather(childEntity, outLights, outProbes);
}

void FacadeRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	Object* renderable
)
{
	auto facadeComponent = mandatory_non_null_type_cast< const FacadeComponent* >(renderable);
	for (auto childEntity : facadeComponent->getVisibleEntities())
		context.setup(worldRenderView, childEntity);
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
	auto facadeComponent = mandatory_non_null_type_cast< const FacadeComponent* >(renderable);
	for (auto childEntity : facadeComponent->getVisibleEntities())
		context.build(worldRenderView, worldRenderPass, childEntity);
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
