#include "Weather/Clouds/CloudComponent.h"
#include "Weather/Clouds/CloudRenderer.h"
#include "World/WorldBuildContext.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.CloudRenderer", CloudRenderer, world::IEntityRenderer)

CloudRenderer::CloudRenderer(render::PrimitiveRenderer* primitiveRenderer)
:	m_primitiveRenderer(primitiveRenderer)
{
}

const TypeInfoSet CloudRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< CloudComponent >();
}

void CloudRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void CloudRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void CloudRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void CloudRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto cloudComponent = mandatory_non_null_type_cast< CloudComponent* >(renderable);
	cloudComponent->build(context.getRenderContext(), worldRenderView, worldRenderPass, m_primitiveRenderer);
}

void CloudRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
