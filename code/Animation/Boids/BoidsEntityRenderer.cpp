#include "Animation/Boids/BoidsEntity.h"
#include "Animation/Boids/BoidsEntityRenderer.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.BoidsEntityRenderer", BoidsEntityRenderer, world::IEntityRenderer)

const TypeInfoSet BoidsEntityRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< BoidsEntity >();
}

void BoidsEntityRenderer::gather(
	world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void BoidsEntityRenderer::build(
	world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	BoidsEntity* boidsEntity = mandatory_non_null_type_cast< BoidsEntity* >(renderable);
	boidsEntity->build(worldContext, worldRenderView, worldRenderPass);
}

void BoidsEntityRenderer::flush(
	world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

void BoidsEntityRenderer::flush(world::WorldContext& worldContext)
{
}

	}
}
