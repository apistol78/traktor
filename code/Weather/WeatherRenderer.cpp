#include "Weather/WeatherRenderer.h"
#include "Weather/Clouds/CloudComponent.h"
#include "Weather/Precipitation/PrecipitationComponent.h"
#include "Weather/Sky/SkyComponent.h"
#include "World/WorldBuildContext.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.WeatherRenderer", WeatherRenderer, world::IEntityRenderer)

WeatherRenderer::WeatherRenderer(render::PrimitiveRenderer* primitiveRenderer)
:	m_primitiveRenderer(primitiveRenderer)
{
}

const TypeInfoSet WeatherRenderer::getRenderableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< CloudComponent >();
	typeSet.insert< PrecipitationComponent >();
	typeSet.insert< SkyComponent >();
	return typeSet;
}

void WeatherRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void WeatherRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (auto cloudComponent = dynamic_type_cast< CloudComponent* >(renderable))
		cloudComponent->build(context.getRenderContext(), worldRenderView, worldRenderPass, m_primitiveRenderer);
	else if (auto precipitationComponent = dynamic_type_cast< PrecipitationComponent* >(renderable))
		precipitationComponent->build(context, worldRenderView, worldRenderPass);
	else if (auto skyComponent = dynamic_type_cast< SkyComponent* >(renderable))
		skyComponent->build(context.getRenderContext(), worldRenderView, worldRenderPass);
}

void WeatherRenderer::flush(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

void WeatherRenderer::setup(const world::WorldSetupContext& context)
{
}

	}
}
