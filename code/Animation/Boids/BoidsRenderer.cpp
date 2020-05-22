#include "Animation/Boids/BoidsComponent.h"
#include "Animation/Boids/BoidsRenderer.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.BoidsRenderer", BoidsRenderer, world::IEntityRenderer)

const TypeInfoSet BoidsRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< BoidsComponent >();
}

void BoidsRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void BoidsRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void BoidsRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void BoidsRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto boidsComponent = mandatory_non_null_type_cast< BoidsComponent* >(renderable);
	boidsComponent->build(context, worldRenderView, worldRenderPass);
}

void BoidsRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
