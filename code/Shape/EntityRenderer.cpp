#include "Shape/EntityRenderer.h"
#include "Shape/Spline/SplineEntity.h"
#include "World/WorldContext.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.EntityRenderer", EntityRenderer, world::IEntityRenderer)

const TypeInfoSet EntityRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< SplineEntity >();
}

void EntityRenderer::gather(
	world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void EntityRenderer::build(
	world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (SplineEntity* splineEntity = dynamic_type_cast< SplineEntity* >(renderable))
	{
		splineEntity->build(
			worldContext,
			worldRenderView,
			worldRenderPass
		);
	}
}

void EntityRenderer::flush(
	world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

void EntityRenderer::flush(world::WorldContext& worldContext)
{
}

	}
}
