#include "Weather/WeatherEntityRenderer.h"
#include "Weather/Clouds/CloudEntity.h"
#include "Weather/Sky/SkyComponent.h"
#include "World/WorldContext.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.WeatherEntityRenderer", WeatherEntityRenderer, world::IEntityRenderer)

WeatherEntityRenderer::WeatherEntityRenderer(render::PrimitiveRenderer* primitiveRenderer)
:	m_primitiveRenderer(primitiveRenderer)
{
}

const TypeInfoSet WeatherEntityRenderer::getRenderableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< CloudEntity >());
	typeSet.insert(&type_of< SkyComponent >());
	return typeSet;
}

void WeatherEntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (CloudEntity* cloudEntity = dynamic_type_cast< CloudEntity* >(renderable))
		cloudEntity->render(worldContext.getRenderContext(), worldRenderView, worldRenderPass, m_primitiveRenderer);
	else if (SkyComponent* skyComponent = dynamic_type_cast< SkyComponent* >(renderable))
		skyComponent->render(worldContext.getRenderContext(), worldRenderView, worldRenderPass);
}

void WeatherEntityRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
