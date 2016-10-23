#include "Weather/WeatherRenderer.h"
#include "Weather/Clouds/CloudComponent.h"
#include "Weather/Precipitation/PrecipitationComponent.h"
#include "Weather/Sky/SkyComponent.h"
#include "World/WorldContext.h"

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
	typeSet.insert(&type_of< CloudComponent >());
	typeSet.insert(&type_of< PrecipitationComponent >());
	typeSet.insert(&type_of< SkyComponent >());
	return typeSet;
}

void WeatherRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (CloudComponent* cloudComponent = dynamic_type_cast< CloudComponent* >(renderable))
		cloudComponent->render(worldContext.getRenderContext(), worldRenderView, worldRenderPass, m_primitiveRenderer);
	else if (PrecipitationComponent* precipitationComponent = dynamic_type_cast< PrecipitationComponent* >(renderable))
		precipitationComponent->render(worldContext.getRenderContext(), worldRenderView, worldRenderPass);
	else if (SkyComponent* skyComponent = dynamic_type_cast< SkyComponent* >(renderable))
		skyComponent->render(worldContext.getRenderContext(), worldRenderView, worldRenderPass);
}

void WeatherRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
