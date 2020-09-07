#include "Weather/Sky/SkyComponent.h"
#include "Weather/Sky/SkyRenderer.h"
#include "World/WorldBuildContext.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.SkyRenderer", SkyRenderer, world::IEntityRenderer)

const TypeInfoSet SkyRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< SkyComponent >();
}

void SkyRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void SkyRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void SkyRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void SkyRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto skyComponent = mandatory_non_null_type_cast< SkyComponent* >(renderable);
	skyComponent->build(context.getRenderContext(), worldRenderView, worldRenderPass);
}

void SkyRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
