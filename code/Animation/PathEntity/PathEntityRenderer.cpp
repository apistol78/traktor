#include "Animation/PathEntity/PathEntity.h"
#include "Animation/PathEntity/PathEntityRenderer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldGatherContext.h"

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
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
	const PathEntity* pathEntity = mandatory_non_null_type_cast< const PathEntity* >(renderable);
	context.gather(pathEntity->getEntity(), outLights);
}

void PathEntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	PathEntity* pathEntity = mandatory_non_null_type_cast< PathEntity* >(renderable);
	context.build(worldRenderView, worldRenderPass, pathEntity->getEntity());
}

void PathEntityRenderer::flush(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

void PathEntityRenderer::flush(const world::WorldBuildContext& context)
{
}

	}
}
