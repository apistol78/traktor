#include "Animation/Boids/BoidsComponent.h"
#include "Animation/Boids/BoidsEntityRenderer.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.BoidsEntityRenderer", BoidsEntityRenderer, world::IEntityRenderer)

const TypeInfoSet BoidsEntityRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< BoidsComponent >();
}

void BoidsEntityRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void BoidsEntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	BoidsComponent* boidsComponent = mandatory_non_null_type_cast< BoidsComponent* >(renderable);
	boidsComponent->build(context, worldRenderView, worldRenderPass);
}

void BoidsEntityRenderer::flush(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

void BoidsEntityRenderer::setup(const world::WorldSetupContext& context)
{
}

	}
}
