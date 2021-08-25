#include "World/WorldGatherContext.h"
#include "World/WorldRenderView.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/LightRenderer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.LightRenderer", LightRenderer, IEntityRenderer)

const TypeInfoSet LightRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< LightComponent >();
}

void LightRenderer::gather(
	const WorldGatherContext& context,
	Object* renderable
)
{
	const LightComponent* lightComponent = static_cast< const LightComponent* >(renderable);
	if (lightComponent->getLightType() != LightType::LtDisabled)
		context.include(this, renderable);
}

void LightRenderer::setup(
	const WorldSetupContext& context
)
{
}

void LightRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void LightRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
}

void LightRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}


	}
}
