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
	AlignedVector< Light >& outLights
)
{
	const LightComponent* lightComponent = mandatory_non_null_type_cast< const LightComponent* >(renderable);
	Transform transform = lightComponent->getTransform();

	Light& light = outLights.push_back();
	light.type = lightComponent->getLightType();
	light.position = transform.translation();
	light.direction = transform.axisY();
	light.color = lightComponent->getColor();
	light.range = lightComponent->getRange();
	light.radius = lightComponent->getRadius();
	light.castShadow = lightComponent->getCastShadow();

	if (lightComponent->getFlickerAmount() > FUZZY_EPSILON)
	{
		Scalar randomFlicker(lightComponent->getFlickerCoeff());
		light.color *= randomFlicker;
	}
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
