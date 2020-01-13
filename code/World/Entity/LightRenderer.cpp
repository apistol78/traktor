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

void LightRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	const LightComponent* lightComponent = mandatory_non_null_type_cast< const LightComponent* >(renderable);
	Transform transform = lightComponent->getTransform();

	Light light;
	light.type = lightComponent->getLightType();
	light.position = transform.translation();
	light.direction = transform.axisY();
	light.color = lightComponent->getColor();
	light.range = Scalar(lightComponent->getRange());
	light.radius = Scalar(lightComponent->getRadius());
	light.castShadow = lightComponent->getCastShadow();

	if (lightComponent->getFlickerAmount() > FUZZY_EPSILON)
	{
		Scalar randomFlicker(lightComponent->getFlickerCoeff());
		light.color *= randomFlicker;
	}

	worldRenderView.addLight(light);
}

void LightRenderer::flush(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

void LightRenderer::flush(WorldContext& worldContext)
{
}

	}
}
