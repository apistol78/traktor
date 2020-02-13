#include "Animation/PathEntity/PathEntity.h"
#include "Animation/PathEntity/PathEntityRenderer.h"
#include "World/WorldContext.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathEntityRenderer", PathEntityRenderer, world::IEntityRenderer)

const TypeInfoSet PathEntityRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< PathEntity >();
}

void PathEntityRenderer::gather(
	const world::WorldContext& worldContext,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
	const PathEntity* pathEntity = mandatory_non_null_type_cast< const PathEntity* >(renderable);
	worldContext.gather(pathEntity->getEntity(), outLights);
}

void PathEntityRenderer::build(
	const world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	PathEntity* pathEntity = mandatory_non_null_type_cast< PathEntity* >(renderable);
	worldContext.build(worldRenderView, worldRenderPass, pathEntity->getEntity());
}

void PathEntityRenderer::flush(
	const world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

void PathEntityRenderer::flush(const world::WorldContext& worldContext)
{
}

	}
}
