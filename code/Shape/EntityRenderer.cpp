#include "Shape/EntityRenderer.h"
#include "Shape/Spline/CloneLayer.h"
#include "Shape/Spline/ExtrudeShapeLayer.h"
#include "World/WorldBuildContext.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.EntityRenderer", EntityRenderer, world::IEntityRenderer)

const TypeInfoSet EntityRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< CloneLayer, ExtrudeShapeLayer >();
}

void EntityRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void EntityRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void EntityRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void EntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (auto cloneLayer = dynamic_type_cast< CloneLayer* >(renderable))
	{
		cloneLayer->build(
			context,
			worldRenderView,
			worldRenderPass
		);
	}
	else if (auto extrudeShapeLayer = dynamic_type_cast< ExtrudeShapeLayer* >(renderable))
	{
		extrudeShapeLayer->build(
			context,
			worldRenderView,
			worldRenderPass
		);
	}
}

void EntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
