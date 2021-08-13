#include "World/WorldBuildContext.h"
#include "World/WorldGatherContext.h"
#include "World/WorldSetupContext.h"
#include "World/Entity.h"
#include "World/Entity/GroupComponent.h"
#include "World/Entity/GroupRenderer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupRenderer", GroupRenderer, IEntityRenderer)

const TypeInfoSet GroupRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< GroupComponent >();
}

void GroupRenderer::gather(
	const WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< const LightComponent* >& outLights,
	AlignedVector< const ProbeComponent* >& outProbes
)
{
	auto groupComponent = static_cast< const GroupComponent* >(renderable);
	for (auto childEntity : groupComponent->getEntities())
		context.gather(childEntity, outLights, outProbes);
}

void GroupRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	Object* renderable
)
{
	auto groupComponent = static_cast< GroupComponent* >(renderable);
	for (auto childEntity : groupComponent->getEntities())
		context.setup(worldRenderView, childEntity);
}

void GroupRenderer::setup(
	const WorldSetupContext& context
)
{
}

void GroupRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto groupComponent = static_cast< GroupComponent* >(renderable);
	for (auto childEntity : groupComponent->getEntities())
		context.build(worldRenderView, worldRenderPass, childEntity);
}

void GroupRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

	}
}
