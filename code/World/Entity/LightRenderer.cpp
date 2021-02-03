#include <limits>
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
	const Object* renderable,
	AlignedVector< const LightComponent* >& outLights,
	AlignedVector< const ProbeComponent* >& outProbes
)
{
	const LightComponent* lightComponent = mandatory_non_null_type_cast< const LightComponent* >(renderable);
	if (lightComponent->getLightType() != LightType::LtDisabled)
		outLights.push_back(lightComponent);
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
