#include "Weather/WeatherEntityRenderer.h"
#include "Weather/Clouds/CloudEntity.h"
#include "Weather/Sky/SkyEntity.h"
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
	typeSet.insert(&type_of< SkyEntity >());
	typeSet.insert(&type_of< CloudEntity >());
	return typeSet;
}

void WeatherEntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (SkyEntity* skyEntity = dynamic_type_cast< SkyEntity* >(renderable))
		skyEntity->render(worldContext.getRenderContext(), worldRenderView, worldRenderPass);
	else if (CloudEntity* cloudEntity = dynamic_type_cast< CloudEntity* >(renderable))
		cloudEntity->render(worldContext.getRenderContext(), worldRenderView, worldRenderPass, m_primitiveRenderer);
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
