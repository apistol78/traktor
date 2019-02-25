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
	return makeTypeInfoSet(
		type_of< SplineEntity >()
	);
}

void EntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (SplineEntity* splineEntity = dynamic_type_cast< SplineEntity* >(renderable))
	{
		splineEntity->render(
			worldContext,
			worldRenderView,
			worldRenderPass
		);
	}
}

void EntityRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
